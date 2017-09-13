#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "isr.h"
#include "mmu.h"

#define GDTADDRESS                      0x1000
#define GDTDESCRIPTORS                  6
#define IDTADDRESS                      0x2000
#define IDTDESCRIPTORS                  256
#define TSSADDRESS                      0x3000
#define TSSDESCRIPTORS                  1
#define KERNELCODEADDRESS               0x00100000
#define KERNELCODESIZE                  0x00300000
#define KERNELSTACKADDRESS              0x00400000
#define KERNELSTACKSIZE                 0x00004000
#define MMUKERNELADDRESS                0x00800000
#define MMUKERNELCOUNT                  32
#define MMUTASKADDRESS                  0x00820000
#define MMUTASKCOUNT                    4
#define TASKCODEADDRESS                 0x01000000
#define TASKCODESIZE                    0x00080000
#define TASKSTACKADDRESS                0x80000000
#define TASKSTACKSIZE                   0x00008000

struct gdt
{

    struct gdt_pointer pointer;
    struct gdt_descriptor descriptors[GDTDESCRIPTORS];

};

struct idt
{

    struct idt_pointer pointer;
    struct idt_descriptor descriptors[IDTDESCRIPTORS];

};

struct tss
{

    struct tss_pointer pointer;
    struct tss_descriptor descriptors[TSSDESCRIPTORS];

};

static struct
{

    unsigned short kcode;
    unsigned short kdata;
    unsigned short ucode;
    unsigned short udata;
    unsigned short tlink;

} selector;

static struct gdt *gdt = (struct gdt *)GDTADDRESS;
static struct idt *idt = (struct idt *)IDTADDRESS;
static struct tss *tss = (struct tss *)TSSADDRESS;
static struct cpu_general registers[KERNEL_TASKS];
static struct arch_context context;

static struct mmu_directory *getdirectory(unsigned int index, unsigned int address, unsigned int count)
{

    return (struct mmu_directory *)address + index * count;

}

static struct mmu_table *gettable(unsigned int index, unsigned int address, unsigned int count)
{

    return (struct mmu_table *)address + index * count + 1;

}

static void copymap(struct task *task)
{

    struct mmu_directory *cdirectory = (struct mmu_directory *)MMUKERNELADDRESS;
    struct mmu_directory *tdirectory = getdirectory(task->id, MMUTASKADDRESS, MMUTASKCOUNT);

    memory_copy(tdirectory, cdirectory, sizeof (struct mmu_directory));

}

static void mapkernel()
{

    struct mmu_directory *directory = (struct mmu_directory *)MMUKERNELADDRESS;
    struct mmu_table *table = (struct mmu_table *)(MMUKERNELADDRESS + 0x1000);

    mmu_map(directory, &table[0], 0x00000000, 0x00000000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(directory, &table[1], 0x00400000, 0x00400000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(directory, &table[2], 0x00800000, 0x00800000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(directory, &table[3], 0x00C00000, 0x00C00000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

static void maptask(struct task *task, unsigned int code)
{

    struct mmu_directory *directory = getdirectory(task->id, MMUTASKADDRESS, MMUTASKCOUNT);
    struct mmu_table *table = gettable(task->id, MMUTASKADDRESS, MMUTASKCOUNT);

    mmu_map(directory, &table[0], TASKCODEADDRESS + task->id * (TASKCODESIZE + TASKSTACKSIZE), code, TASKCODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_map(directory, &table[1], TASKCODEADDRESS + task->id * (TASKCODESIZE + TASKSTACKSIZE) + TASKCODESIZE, TASKSTACKADDRESS - TASKSTACKSIZE, TASKSTACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static void activate(struct task *task)
{

    struct mmu_directory *directory = getdirectory(task->id, MMUTASKADDRESS, MMUTASKCOUNT);

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

static unsigned int spawn(struct task *task, void *stack)
{

    struct task *next = kernel_getfreetask();

    if (!next)
        return 0;

    copymap(next);
    kernel_copyservices(task, next);

    if (!kernel_setupbinary(next, TASKSTACKADDRESS))
        return 0;

    kernel_activatetask(next);

    return 1;

}

static unsigned int despawn(struct task *task, void *stack)
{

    kernel_freetask(task);

    return 1;

}

void arch_setinterrupt(unsigned char index, void (*callback)(void))
{

    idt_setdescriptor(&idt->pointer, index, callback, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

}

void arch_setmap(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = (struct mmu_directory *)MMUKERNELADDRESS;
    struct mmu_table *table = (struct mmu_table *)(MMUKERNELADDRESS + 0x1000);

    mmu_map(directory, &table[index], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_setdirectory(directory);

}

unsigned int arch_call(unsigned int index, void *stack, unsigned int rewind)
{

    context.task->state.rewind = rewind;

    return abi_call(index, context.task, stack);

}

struct arch_context *arch_schedule(struct cpu_general *general, unsigned int ip, unsigned int sp)
{

    if (context.task)
    {

        context.task->state.ip = ip;
        context.task->state.sp = sp;

        saveregisters(context.task, general);

    }

    context.task = kernel_getactivetask();

    if (context.task)
    {

        if (context.task->state.status == TASK_STATUS_UNBLOCKED)
        {

            kernel_activatetask(context.task);
            task_setstate(context.task, context.task->state.ip - context.task->state.rewind, context.task->state.sp);

        }

        loadregisters(context.task, general);
        activate(context.task);

    }

    return &context;

}

unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct arch_context *context = arch_schedule(general, interrupt->eip.value, interrupt->esp.value);

    if (context->task)
    {

        interrupt->cs.value = selector.ucode;
        interrupt->ss.value = selector.udata;
        interrupt->eip.value = context->task->state.ip;
        interrupt->esp.value = context->task->state.sp;

    }

    else
    {

        interrupt->cs.value = selector.kcode;
        interrupt->ss.value = selector.kdata;
        interrupt->eip.value = context->ip;
        interrupt->esp.value = context->sp;

    }

    return interrupt->ss.value;

}

unsigned short arch_zero(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG(DEBUG_INFO, "exception: divide by zero");

    if (interrupt.cs.value == selector.ucode)
        kernel_freetask(context.task);

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

    if (context.task)
    {

        unsigned int code = context.task->format->findbase(&context.task->node, cpu_getcr2());

        if (code)
        {

            maptask(context.task, code);
            context.task->format->copyprogram(&context.task->node);

        }

        else
        {

            kernel_freetask(context.task);

        }

    }

    return arch_resume(&general, &interrupt);

}

unsigned short arch_syscall(struct cpu_general general, struct cpu_interrupt interrupt)
{

    general.eax.value = arch_call(general.eax.value, interrupt.esp.reference, 7);

    return arch_resume(&general, &interrupt);

}

void arch_leave(unsigned short code, unsigned short data, unsigned int ip, unsigned int sp)
{

    struct cpu_interrupt interrupt;

    interrupt.cs.value = code;
    interrupt.ss.value = data;
    interrupt.eip.value = ip;
    interrupt.esp.value = sp;
    interrupt.eflags.value = cpu_geteflags() | CPU_FLAGS_IF;

    cpu_leave(interrupt);

}

void arch_setup(struct service_backend *backend)
{

    gdt_initpointer(&gdt->pointer, GDTDESCRIPTORS, gdt->descriptors);
    idt_initpointer(&idt->pointer, IDTDESCRIPTORS, idt->descriptors);
    tss_initpointer(&tss->pointer, TSSDESCRIPTORS, tss->descriptors);

    selector.kcode = gdt_setdescriptor(&gdt->pointer, 0x01, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.kdata = gdt_setdescriptor(&gdt->pointer, 0x02, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.ucode = gdt_setdescriptor(&gdt->pointer, 0x03, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.udata = gdt_setdescriptor(&gdt->pointer, 0x04, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.tlink = gdt_setdescriptor(&gdt->pointer, 0x05, (unsigned int)tss->pointer.descriptors, (unsigned int)tss->pointer.descriptors + tss->pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);

    idt_setdescriptor(&idt->pointer, 0x00, isr_zero, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x01, isr_debug, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x02, isr_nmi, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x03, isr_breakpoint, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    idt_setdescriptor(&idt->pointer, 0x04, isr_overflow, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x05, isr_bound, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x06, isr_opcode, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x07, isr_device, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x08, isr_doublefault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0A, isr_tss, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0B, isr_segment, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0C, isr_stack, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0D, isr_generalfault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0E, isr_pagefault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x80, isr_syscall, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    tss_setdescriptor(&tss->pointer, 0x00, selector.kdata, KERNELSTACKADDRESS + KERNELSTACKSIZE);
    cpu_setgdt(&gdt->pointer, selector.kcode, selector.kdata);
    cpu_setidt(&idt->pointer);
    cpu_settss(selector.tlink);
    abi_setup(spawn, despawn);
    binary_setupelf();
    service_setupcpio();
    kernel_setuptasks();
    kernel_setupservers();
    kernel_setupmounts();
    kernel_setupservices();

    context.task = kernel_getfreetask();
    context.ip = (unsigned int)cpu_halt;
    context.sp = KERNELSTACKADDRESS + KERNELSTACKSIZE;

    kernel_setupramdisk(context.task, backend);
    mapkernel();
    spawn(context.task, 0);
    activate(context.task);
    mmu_enable();
    arch_leave(selector.ucode, selector.udata, context.task->state.ip, context.task->state.sp);

}

