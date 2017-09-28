#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "isr.h"
#include "mmu.h"

struct gdt
{

    struct gdt_pointer pointer;
    struct gdt_descriptor descriptors[ARCH_GDTDESCRIPTORS];

};

struct idt
{

    struct idt_pointer pointer;
    struct idt_descriptor descriptors[ARCH_IDTDESCRIPTORS];

};

struct tss
{

    struct tss_pointer pointer;
    struct tss_descriptor descriptors[ARCH_TSSDESCRIPTORS];

};

static struct gdt *gdt = (struct gdt *)ARCH_GDTADDRESS;
static struct idt *idt = (struct idt *)ARCH_IDTADDRESS;
static struct cpu_general registers[KERNEL_TASKS];
static struct arch_context context0;
static struct tss tss0;

static struct mmu_directory *getdirectory(unsigned int index)
{

    return (struct mmu_directory *)ARCH_MMUTASKADDRESS + index * ARCH_MMUTASKCOUNT;

}

static struct mmu_table *gettable(unsigned int index)
{

    return (struct mmu_table *)ARCH_MMUTASKADDRESS + index * ARCH_MMUTASKCOUNT + 1;

}

static void mapkernel(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = (struct mmu_directory *)ARCH_MMUKERNELADDRESS;
    struct mmu_table *table = (struct mmu_table *)(ARCH_MMUKERNELADDRESS + 0x1000);

    mmu_map(directory, &table[index], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

static void maptask(struct task *task, unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = getdirectory(task->id);
    struct mmu_table *table = gettable(task->id);

    mmu_map(directory, &table[index], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static unsigned int spawn(struct task *task, void *stack)
{

    struct arch_context *context = arch_getcontext();
    struct task *next = kernel_getfreetask();

    if (!next)
        return 0;

    memory_copy(getdirectory(next->id), (struct mmu_directory *)ARCH_MMUKERNELADDRESS, sizeof (struct mmu_directory));
    kernel_copyservices(task, next);

    if (!kernel_setupbinary(next, ARCH_TASKSTACKADDRESS))
        return 0;

    kernel_assigntask(&context->core, next);

    return 1;

}

static unsigned int despawn(struct task *task, void *stack)
{

    kernel_freetask(task);

    return 1;

}

struct arch_context *arch_getcontext(void)
{

    return &context0;

}

void arch_setmap(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    mapkernel(index, paddress, vaddress, size);
    mmu_setdirectory((struct mmu_directory *)ARCH_MMUKERNELADDRESS);

}

void arch_schedule(struct cpu_general *general, struct arch_context *context, unsigned int ip, unsigned int sp)
{

    if (context->task)
    {

        context->task->state.ip = ip;
        context->task->state.sp = sp;

        memory_copy(&registers[context->task->id], general, sizeof (struct cpu_general));

    }

    context->task = kernel_schedule(&context->core);

    if (context->task)
    {

        memory_copy(general, &registers[context->task->id], sizeof (struct cpu_general));
        mmu_setdirectory(getdirectory(context->task->id));

    }

}

unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct arch_context *context = arch_getcontext();

    arch_schedule(general, context, interrupt->eip.value, interrupt->esp.value);

    if (context->task)
    {

        interrupt->cs.value = gdt_getselector(&gdt->pointer, 3);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, 4);
        interrupt->eip.value = context->task->state.ip;
        interrupt->esp.value = context->task->state.sp;

    }

    else
    {

        interrupt->cs.value = gdt_getselector(&gdt->pointer, 1);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, 2);
        interrupt->eip.value = context->ip;
        interrupt->esp.value = context->sp;

    }

    return interrupt->ss.value;

}

unsigned short arch_zero(struct cpu_general general, struct cpu_interrupt interrupt)
{

    struct arch_context *context = arch_getcontext();

    DEBUG(DEBUG_INFO, "exception: divide by zero");

    if (interrupt.cs.value == gdt_getselector(&gdt->pointer, 3))
        kernel_freetask(context->task);

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

unsigned short arch_doublefault(struct cpu_general general, unsigned int zero, struct cpu_interrupt interrupt)
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

    struct arch_context *context = arch_getcontext();

    if (context->task)
    {

        unsigned int code = context->task->format->findbase(&context->task->node, cpu_getcr2());

        if (code)
        {

            maptask(context->task, 0, ARCH_TASKCODEADDRESS + context->task->id * (ARCH_TASKCODESIZE + ARCH_TASKSTACKSIZE), code, ARCH_TASKCODESIZE);
            maptask(context->task, 1, ARCH_TASKCODEADDRESS + context->task->id * (ARCH_TASKCODESIZE + ARCH_TASKSTACKSIZE) + ARCH_TASKCODESIZE, ARCH_TASKSTACKADDRESS - ARCH_TASKSTACKSIZE, ARCH_TASKSTACKSIZE);
            context->task->format->copyprogram(&context->task->node);

        }

        else
        {

            kernel_freetask(context->task);

        }

    }

    return arch_resume(&general, &interrupt);

}

unsigned short arch_syscall(struct cpu_general general, struct cpu_interrupt interrupt)
{

    struct arch_context *context = arch_getcontext();

    context->task->state.rewind = 7;
    general.eax.value = abi_call(general.eax.value, context->task, interrupt.esp.reference);

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

void arch_initcontext(struct arch_context *context, unsigned int id, unsigned int ip, unsigned int sp)
{

    core_init(&context->core, id);

    context->ip = ip;
    context->sp = sp;

}

void arch_setup(struct service_backend *backend)
{

    arch_initcontext(&context0, 0, (unsigned int)cpu_halt, ARCH_KERNELSTACKADDRESS + ARCH_KERNELSTACKSIZE);
    gdt_initpointer(&gdt->pointer, ARCH_GDTDESCRIPTORS, gdt->descriptors);
    gdt_setdescriptor(&gdt->pointer, 0x01, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, 0x02, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, 0x03, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, 0x04, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    cpu_setgdt(&gdt->pointer, gdt_getselector(&gdt->pointer, 1), gdt_getselector(&gdt->pointer, 2));
    idt_initpointer(&idt->pointer, ARCH_IDTDESCRIPTORS, idt->descriptors);
    idt_setdescriptor(&idt->pointer, 0x00, isr_zero, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x01, isr_debug, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x02, isr_nmi, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x03, isr_breakpoint, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    idt_setdescriptor(&idt->pointer, 0x04, isr_overflow, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x05, isr_bound, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x06, isr_opcode, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x07, isr_device, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0A, isr_tss, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0B, isr_segment, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0C, isr_stack, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0D, isr_generalfault, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0E, isr_pagefault, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x80, isr_syscall, gdt_getselector(&gdt->pointer, 1), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    cpu_setidt(&idt->pointer);
    tss_initpointer(&tss0.pointer, ARCH_TSSDESCRIPTORS, tss0.descriptors);
    tss_setdescriptor(&tss0.pointer, 0x00, gdt_getselector(&gdt->pointer, 2), ARCH_KERNELSTACKADDRESS + ARCH_KERNELSTACKSIZE);
    gdt_setdescriptor(&gdt->pointer, 0x05, (unsigned int)tss0.pointer.descriptors, (unsigned int)tss0.pointer.descriptors + tss0.pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);
    cpu_settss(gdt_getselector(&gdt->pointer, 5));
    abi_setup(spawn, despawn);
    binary_setupelf();
    service_setupcpio();
    kernel_setuptasks();
    kernel_setupservers();
    kernel_setupmounts();
    kernel_setupservices();

    context0.task = kernel_getfreetask();

    kernel_setupramdisk(context0.task, backend);
    mapkernel(0, 0x00000000, 0x00000000, 0x00400000);
    mapkernel(1, 0x00400000, 0x00400000, 0x00400000);
    mapkernel(2, 0x00800000, 0x00800000, 0x00400000);
    mapkernel(3, 0x00C00000, 0x00C00000, 0x00400000);
    spawn(context0.task, 0);
    mmu_setdirectory(getdirectory(context0.task->id));
    mmu_enable();
    arch_leave(gdt_getselector(&gdt->pointer, 3), gdt_getselector(&gdt->pointer, 4), context0.task->state.ip, context0.task->state.sp);

}

