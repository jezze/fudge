#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "isr.h"
#include "mmu.h"

#define CONTAINERS                      16
#define TASKS                           128
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

static struct arch_container
{

    struct container base;
    struct mmu_directory *directory;
    struct mmu_table *table;

} containers[CONTAINERS];

static struct arch_task
{

    struct task base;
    struct cpu_general general;
    unsigned int code;
    unsigned int stack;
    struct mmu_directory *directory;
    struct mmu_table *table;

} tasks[TASKS];

static struct arch_context current;

static void copymap(struct container *container, struct task *task)
{

    struct arch_task *atask = (struct arch_task *)task;
    struct arch_container *acontainer = (struct arch_container *)container;

    memory_copy(atask->directory, acontainer->directory, sizeof (struct mmu_directory));

}

static void mapcontainer(struct container *container)
{

    struct arch_container *acontainer = (struct arch_container *)container;

    mmu_map(acontainer->directory, &acontainer->table[0], 0x00000000, 0x00000000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(acontainer->directory, &acontainer->table[1], 0x00400000, 0x00400000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(acontainer->directory, &acontainer->table[2], 0x00800000, 0x00800000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(acontainer->directory, &acontainer->table[3], 0x00C00000, 0x00C00000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

static void maptask(struct task *task, unsigned int code, unsigned int stack)
{

    struct arch_task *atask = (struct arch_task *)task;

    mmu_map(atask->directory, &atask->table[0], atask->code, code, CODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_map(atask->directory, &atask->table[1], atask->stack, stack, STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static void activate(struct task *task)
{

    struct arch_task *atask = (struct arch_task *)task;

    mmu_setdirectory(atask->directory);

}

static void saveregisters(struct task *task, struct cpu_general *general)
{

    struct arch_task *atask = (struct arch_task *)task;

    memory_copy(&atask->general, general, sizeof (struct cpu_general));

}

static void loadregisters(struct task *task, struct cpu_general *general)
{

    struct arch_task *atask = (struct arch_task *)task;

    memory_copy(general, &atask->general, sizeof (struct cpu_general));

}

static unsigned int spawn(struct container *container, struct task *task, void *stack)
{

    struct task *next = task_findinactive();

    if (!next)
        return 0;

    copymap(container, next);
    kernel_copydescriptors(task, next);

    return kernel_setupbinary(next, TASKSTACK);

}

static unsigned int despawn(struct container *container, struct task *task, void *stack)
{

    task_setstatus(task, TASK_STATUS_INACTIVE);

    return 1;

}

void arch_setinterrupt(unsigned char index, void (*callback)(void))
{

    idt_setdescriptor(&idt.pointer, index, callback, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

}

void arch_setmap(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct arch_container *acontainer = (struct arch_container *)current.container;

    mmu_map(acontainer->directory, &acontainer->table[index], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_setdirectory(acontainer->directory);

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

    current.task = task_findactive();

    if (current.task)
    {

        if (current.task->state.status == TASK_STATUS_UNBLOCKED)
            task_resume(current.task, current.task->state.ip - current.task->state.rewind, current.task->state.sp);

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
        task_setstatus(current.task, TASK_STATUS_INACTIVE);

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

    unsigned int address = cpu_getcr2();

    DEBUG(DEBUG_INFO, "exception: page fault");

    if (current.task)
    {

        address = current.task->format->findbase(&current.task->node, address);

        if (address)
        {

            maptask(current.task, address, TASKSTACK - STACKSIZE);
            current.task->format->copyprogram(&current.task->node);

        }

        else
        {

            task_setstatus(current.task, TASK_STATUS_INACTIVE);

        }

    }

    return arch_resume(&general, &interrupt);

}

unsigned short arch_syscall(struct cpu_general general, struct cpu_interrupt interrupt)
{

    general.eax.value = arch_call(general.eax.value, interrupt.esp.reference, 7);

    return arch_resume(&general, &interrupt);

}

static struct container *setupcontainers(unsigned int count)
{

    unsigned int index;

    for (index = 0; index < count; index++)
    {

        struct arch_container *container = &containers[index];

        container_init(&container->base);

        container->directory = (struct mmu_directory *)CONTAINERMMUBASE + index * CONTAINERMMUCOUNT;
        container->table = (struct mmu_table *)CONTAINERMMUBASE + index * CONTAINERMMUCOUNT + 1;

    }

    return &containers[0].base;

}

static struct task *setuptasks(unsigned int count)
{

    unsigned int index;

    for (index = 0; index < count; index++)
    {

        struct arch_task *task = &tasks[index];

        task_init(&task->base, index);

        task->code = PHYSBASE + index * (CODESIZE + STACKSIZE);
        task->stack = task->code + CODESIZE;
        task->directory = (struct mmu_directory *)TASKMMUBASE + index * TASKMMUCOUNT;
        task->table = (struct mmu_table *)TASKMMUBASE + index * TASKMMUCOUNT + 1;

        task_register(&task->base);

    }

    return &tasks[0].base;

}

static void setupdescriptors()
{

    unsigned int i;

    for (i = 0; i < TASKS; i++)
    {

        struct arch_task *task = &tasks[i];
        unsigned int j;

        for (j = 0; j < TASK_DESCRIPTORS; j++)
            task_initdescriptor(&task->base.descriptors[j], &task->base);

    }

}

void arch_setup(struct service_backend *backend)
{

    struct cpu_interrupt interrupt;

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
    kernel_setup();

    current.container = setupcontainers(CONTAINERS);
    current.task = setuptasks(TASKS);
    current.ip = (unsigned int)cpu_halt;
    current.sp = KERNELSTACK;

    setupdescriptors();
    kernel_setupramdisk(current.container, current.task, backend);
    mapcontainer(current.container);
    copymap(current.container, current.task);
    kernel_copydescriptors(current.task, current.task);
    kernel_setupbinary(current.task, TASKSTACK);
    activate(current.task);
    mmu_setup();
    abi_setup(spawn, despawn);

    interrupt.cs.value = selector.ucode;
    interrupt.ss.value = selector.ustack;
    interrupt.eip.value = current.task->state.ip;
    interrupt.esp.value = current.task->state.sp;
    interrupt.eflags.value = cpu_geteflags() | CPU_FLAGS_IF;

    cpu_leave(interrupt);

}

