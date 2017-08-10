#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "isr.h"
#include "mmu.h"

#define GDTDESCRIPTORS                  6
#define IDTDESCRIPTORS                  256
#define TSSDESCRIPTORS                  1
#define KERNELSTACK                     0x00400000
#define TASKSTACK                       0x80000000
#define CONTAINERMMUBASE                0x00400000
#define CONTAINERMMUCOUNT               8
#define TASKMMUBASE                     0x00480000
#define TASKMMUCOUNT                    4
#define PHYSBASE                        0x01000000
#define CODESIZE                        0x00080000
#define STACKSIZE                       0x00008000

static struct
{

    struct gdt_pointer pointer;
    struct gdt_descriptor descriptors[GDTDESCRIPTORS];

} gdt;

static struct
{

    struct idt_pointer pointer;
    struct idt_descriptor descriptors[IDTDESCRIPTORS];

} idt;

static struct
{

    struct tss_pointer pointer;
    struct tss_descriptor descriptors[TSSDESCRIPTORS];

} tss;

static struct
{

    unsigned short kcode;
    unsigned short kstack;
    unsigned short ucode;
    unsigned short ustack;
    unsigned short tlink;

} selector;

static struct cpu_general registers[KERNEL_TASKS];
static struct arch_context current;

static struct mmu_directory *getdirectory(unsigned int index, unsigned int base, unsigned int count)
{

    return (struct mmu_directory *)base + index * count;

}

static struct mmu_table *gettable(unsigned int index, unsigned int base, unsigned int count)
{

    return (struct mmu_table *)base + index * count + 1;

}

static void copymap(struct container *container, struct task *task)
{

    struct mmu_directory *cdirectory = getdirectory(container->id, CONTAINERMMUBASE, CONTAINERMMUCOUNT);
    struct mmu_directory *tdirectory = getdirectory(task->id, TASKMMUBASE, TASKMMUCOUNT);

    memory_copy(tdirectory, cdirectory, sizeof (struct mmu_directory));

}

static void mapcontainer(struct container *container)
{

    struct mmu_directory *directory = getdirectory(container->id, CONTAINERMMUBASE, CONTAINERMMUCOUNT);
    struct mmu_table *table = gettable(container->id, CONTAINERMMUBASE, CONTAINERMMUCOUNT);

    mmu_map(directory, &table[0], 0x00000000, 0x00000000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(directory, &table[1], 0x00400000, 0x00400000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(directory, &table[2], 0x00800000, 0x00800000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(directory, &table[3], 0x00C00000, 0x00C00000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

static void maptask(struct task *task, unsigned int code)
{

    struct mmu_directory *directory = getdirectory(task->id, TASKMMUBASE, TASKMMUCOUNT);
    struct mmu_table *table = gettable(task->id, TASKMMUBASE, TASKMMUCOUNT);

    mmu_map(directory, &table[0], PHYSBASE + task->id * (CODESIZE + STACKSIZE), code, CODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_map(directory, &table[1], PHYSBASE + task->id * (CODESIZE + STACKSIZE) + CODESIZE, TASKSTACK - STACKSIZE, STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static void activate(struct task *task)
{

    struct mmu_directory *directory = getdirectory(task->id, TASKMMUBASE, TASKMMUCOUNT);

    mmu_setdirectory(directory);

}

static void saveregisters(struct task *task, struct cpu_general *general)
{

    memory_copy(&registers[task->id], general, sizeof (struct cpu_general));

}

static void loadregisters(struct task *task, struct cpu_general *general)
{

    memory_copy(general, &registers[task->id], sizeof (struct cpu_general));

}

static unsigned int spawn(struct container *container, struct task *task, void *stack)
{

    struct task *next = kernel_getinactivetask();

    if (!next)
        return 0;

    copymap(container, next);
    kernel_copyservices(task, next);

    if (!kernel_setupbinary(next, TASKSTACK))
        return 0;

    kernel_activatetask(next);

    return 1;

}

static unsigned int despawn(struct container *container, struct task *task, void *stack)
{

    kernel_inactivatetask(task);

    return 1;

}

void arch_setinterrupt(unsigned char index, void (*callback)(void))
{

    idt_setdescriptor(&idt.pointer, index, callback, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

}

void arch_setmap(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = getdirectory(current.container->id, CONTAINERMMUBASE, CONTAINERMMUCOUNT);
    struct mmu_table *table = gettable(current.container->id, CONTAINERMMUBASE, CONTAINERMMUCOUNT);

    mmu_map(directory, &table[index], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_setdirectory(directory);

}

unsigned int arch_call(unsigned int index, void *stack, unsigned int rewind)
{

    current.task->state.rewind = rewind;

    return abi_call(index, current.container, current.task, stack);

}

struct arch_context *arch_schedule(struct cpu_general *general, unsigned int ip, unsigned int sp)
{

    if (current.task)
    {

        current.task->state.ip = ip;
        current.task->state.sp = sp;

        saveregisters(current.task, general);

    }

    current.task = kernel_getactivetask();

    if (current.task)
    {

        if (current.task->state.status == TASK_STATUS_UNBLOCKED)
        {

            kernel_activatetask(current.task);
            task_setstate(current.task, current.task->state.ip - current.task->state.rewind, current.task->state.sp);

        }

        loadregisters(current.task, general);
        activate(current.task);

    }

    return &current;

}

unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct arch_context *context = arch_schedule(general, interrupt->eip.value, interrupt->esp.value);

    if (context->task)
    {

        interrupt->cs.value = selector.ucode;
        interrupt->ss.value = selector.ustack;
        interrupt->eip.value = context->task->state.ip;
        interrupt->esp.value = context->task->state.sp;

    }

    else
    {

        interrupt->cs.value = selector.kcode;
        interrupt->ss.value = selector.kstack;
        interrupt->eip.value = context->ip;
        interrupt->esp.value = context->sp;

    }

    return interrupt->ss.value;

}

unsigned short arch_zero(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: divide by zero");

    if (interrupt.cs.value == selector.ucode)
        kernel_inactivatetask(current.task);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_debug(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: debug");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_nmi(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: non-maskable interrupt");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_breakpoint(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: breakpoint");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_overflow(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: overflow");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_bound(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: bound range exceeded");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_opcode(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: invalid opcode");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_device(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: device unavailable");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_doublefault(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: double fault");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_tss(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: invalid tss");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_segment(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: segment not present");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_stack(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: stack segment fault");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_generalfault(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: general fault");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_pagefault(struct cpu_general general, unsigned int type, struct cpu_interrupt interrupt)
{

    if (current.task)
    {

        unsigned int code = current.task->format->findbase(&current.task->node, cpu_getcr2());

        if (code)
        {

            maptask(current.task, code);
            current.task->format->copyprogram(&current.task->node);

        }

        else
        {

            kernel_inactivatetask(current.task);

        }

    }

    return arch_resume(&general, &interrupt);

}

unsigned short arch_syscall(struct cpu_general general, struct cpu_interrupt interrupt)
{

    general.eax.value = arch_call(general.eax.value, interrupt.esp.reference, 7);

    return arch_resume(&general, &interrupt);

}

static void leave(void)
{

    struct cpu_interrupt interrupt;

    interrupt.cs.value = selector.ucode;
    interrupt.ss.value = selector.ustack;
    interrupt.eip.value = current.task->state.ip;
    interrupt.esp.value = current.task->state.sp;
    interrupt.eflags.value = cpu_geteflags() | CPU_FLAGS_IF;

    cpu_leave(interrupt);

}

void arch_setup(struct service_backend *backend)
{

    gdt_initpointer(&gdt.pointer, GDTDESCRIPTORS, gdt.descriptors);
    idt_initpointer(&idt.pointer, IDTDESCRIPTORS, idt.descriptors);
    tss_initpointer(&tss.pointer, TSSDESCRIPTORS, tss.descriptors);

    selector.kcode = gdt_setdescriptor(&gdt.pointer, 0x01, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.kstack = gdt_setdescriptor(&gdt.pointer, 0x02, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.ucode = gdt_setdescriptor(&gdt.pointer, 0x03, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.ustack = gdt_setdescriptor(&gdt.pointer, 0x04, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.tlink = gdt_setdescriptor(&gdt.pointer, 0x05, (unsigned int)tss.pointer.descriptors, (unsigned int)tss.pointer.descriptors + tss.pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);

    idt_setdescriptor(&idt.pointer, 0x00, isr_zero, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x01, isr_debug, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x02, isr_nmi, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x03, isr_breakpoint, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    idt_setdescriptor(&idt.pointer, 0x04, isr_overflow, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x05, isr_bound, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x06, isr_opcode, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x07, isr_device, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x08, isr_doublefault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x0A, isr_tss, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x0B, isr_segment, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x0C, isr_stack, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x0D, isr_generalfault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x0E, isr_pagefault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, 0x80, isr_syscall, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    tss_setdescriptor(&tss.pointer, 0x00, selector.kstack, KERNELSTACK);
    cpu_setgdt(&gdt.pointer, selector.kcode, selector.kstack);
    cpu_setidt(&idt.pointer);
    cpu_settss(selector.tlink);
    abi_setup(spawn, despawn);
    binary_setupelf();
    service_setupcpio();

    current.container = kernel_setupcontainers();
    current.task = kernel_setuptasks();
    current.ip = (unsigned int)cpu_halt;
    current.sp = KERNELSTACK;

    kernel_setupservers();
    kernel_setupmounts();
    kernel_setupservices();
    kernel_setupramdisk(current.container, current.task, backend);
    mapcontainer(current.container);
    spawn(current.container, current.task, 0);
    activate(current.task);
    mmu_setup();
    leave();

}

