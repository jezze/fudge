#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "isr.h"
#include "mmu.h"
#include "arch.h"

static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTADDRESS;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTADDRESS;
static struct cpu_general registers[KERNEL_TASKS];
static struct core core0;
static struct arch_tss tss0;

static struct mmu_directory *getkerneldirectory(void)
{

    return (struct mmu_directory *)ARCH_MMUKERNELADDRESS;

}

static struct mmu_directory *gettaskdirectory(unsigned int index)
{

    return (struct mmu_directory *)ARCH_MMUTASKADDRESS + index * ARCH_MMUTASKCOUNT;

}

static void mapkernel(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = getkerneldirectory();
    struct mmu_table *table = (struct mmu_table *)(directory + 1) + index;

    mmu_map(directory, &table[index], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

static void maptask(struct task *task, unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = gettaskdirectory(task->id);
    struct mmu_table *table = (struct mmu_table *)(directory + 1) + index;

    mmu_map(directory, &table[index], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static unsigned int spawn(struct task *task, void *stack)
{

    struct task *next = kernel_picktask();

    if (!next)
        return 0;

    memory_copy(gettaskdirectory(next->id), getkerneldirectory(), sizeof (struct mmu_directory));
    kernel_copyservices(task, next);

    if (kernel_setupbinary(next, ARCH_TASKSTACKADDRESS))
    {

        kernel_usetask(next);

        return 1;

    }

    else
    {

        kernel_freetask(next);

        return 0;

    }

}

static unsigned int despawn(struct task *task, void *stack)
{

    kernel_freetask(task);

    return 1;

}

static struct core *coreget(void)
{

    return &core0;

}

static void coreassign(struct task *task)
{

    list_add(&core0.tasks, &task->item);

}

void arch_setmap(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    mapkernel(index, paddress, vaddress, size);
    mmu_setdirectory(getkerneldirectory());

}

static void schedule(struct cpu_general *general, struct cpu_interrupt *interrupt, struct core *core)
{

    if (core->task)
    {

        memory_copy(&registers[core->task->id], general, sizeof (struct cpu_general));

        core->task->thread.ip = interrupt->eip.value;
        core->task->thread.sp = interrupt->esp.value;

    }

    kernel_schedule(core);

    if (core->task)
    {

        memory_copy(general, &registers[core->task->id], sizeof (struct cpu_general));

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_UCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_UDATA);
        interrupt->eip.value = core->task->thread.ip;
        interrupt->esp.value = core->task->thread.sp;

        mmu_setdirectory(gettaskdirectory(core->task->id));

    }

    else
    {

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_KCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_KDATA);
        interrupt->eip.value = (unsigned int)cpu_halt;
        interrupt->esp.value = core->sp;

    }

}

unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    schedule(general, interrupt, core);

    return interrupt->ss.value;

}

void arch_leave(struct core *core)
{

    struct cpu_general general;
    struct cpu_interrupt interrupt;

    interrupt.eflags.value = cpu_geteflags() | CPU_FLAGS_IF;

    schedule(&general, &interrupt, core);
    cpu_leave(interrupt);

}

unsigned short arch_zero(struct cpu_general general, struct cpu_interrupt interrupt)
{

    struct core *core = kernel_getcore();

    DEBUG_LOG(DEBUG_INFO, "exception: divide by zero");

    if (interrupt.cs.value == gdt_getselector(&gdt->pointer, ARCH_UCODE))
        kernel_freetask(core->task);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_debug(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: debug");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_nmi(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: non-maskable interrupt");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_breakpoint(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: breakpoint");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_overflow(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: overflow");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_bound(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: bound range exceeded");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_opcode(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: invalid opcode");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_device(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: device unavailable");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_doublefault(struct cpu_general general, unsigned int zero, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: double fault");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_tss(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: invalid tss");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_segment(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: segment not present");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_stack(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: stack segment fault");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_generalfault(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_LOG(DEBUG_INFO, "exception: general fault");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_pagefault(struct cpu_general general, unsigned int type, struct cpu_interrupt interrupt)
{

    struct core *core = kernel_getcore();

    if (core->task)
    {

        unsigned int code = core->task->format->findbase(&core->task->node, cpu_getcr2());

        if (code)
        {

            maptask(core->task, 0, ARCH_TASKCODEADDRESS + core->task->id * (ARCH_TASKCODESIZE + ARCH_TASKSTACKSIZE), code, ARCH_TASKCODESIZE);
            maptask(core->task, 1, ARCH_TASKCODEADDRESS + core->task->id * (ARCH_TASKCODESIZE + ARCH_TASKSTACKSIZE) + ARCH_TASKCODESIZE, ARCH_TASKSTACKADDRESS - ARCH_TASKSTACKSIZE, ARCH_TASKSTACKSIZE);
            core->task->format->copyprogram(&core->task->node);

        }

        else
        {

            kernel_freetask(core->task);

        }

    }

    return arch_resume(&general, &interrupt);

}

unsigned short arch_syscall(struct cpu_general general, struct cpu_interrupt interrupt)
{

    struct core *core = kernel_getcore();

    core->task->thread.rewind = 7;
    general.eax.value = abi_call(general.eax.value, core->task, interrupt.esp.reference);

    return arch_resume(&general, &interrupt);

}

void arch_configuregdt(void)
{

    gdt_initpointer(&gdt->pointer, ARCH_GDTDESCRIPTORS, gdt->descriptors);
    gdt_cleardescriptors(&gdt->pointer, ARCH_GDTDESCRIPTORS);
    gdt_setdescriptor(&gdt->pointer, ARCH_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, ARCH_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, ARCH_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, ARCH_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    cpu_setgdt(&gdt->pointer, gdt_getselector(&gdt->pointer, ARCH_KCODE), gdt_getselector(&gdt->pointer, ARCH_KDATA));

}

void arch_configureidt(void)
{

    idt_initpointer(&idt->pointer, ARCH_IDTDESCRIPTORS, idt->descriptors);
    idt_cleardescriptors(&idt->pointer, ARCH_IDTDESCRIPTORS);
    idt_setdescriptor(&idt->pointer, 0x00, isr_zero, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x01, isr_debug, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x02, isr_nmi, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x03, isr_breakpoint, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    idt_setdescriptor(&idt->pointer, 0x04, isr_overflow, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x05, isr_bound, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x06, isr_opcode, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x07, isr_device, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0A, isr_tss, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0B, isr_segment, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0C, isr_stack, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0D, isr_generalfault, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0E, isr_pagefault, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x80, isr_syscall, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    cpu_setidt(&idt->pointer);

}

void arch_configuretss(struct arch_tss *tss, unsigned int id, unsigned int sp)
{

    tss_initpointer(&tss->pointer, ARCH_TSSDESCRIPTORS, tss->descriptors);
    tss_setdescriptor(&tss->pointer, 0, gdt_getselector(&gdt->pointer, ARCH_KDATA), sp);
    gdt_setdescriptor(&gdt->pointer, id, (unsigned int)tss->pointer.descriptors, (unsigned int)tss->pointer.descriptors + tss->pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);
    cpu_settss(gdt_getselector(&gdt->pointer, id));

}

static void setuptask()
{

    struct task *task = kernel_picktask();

    kernel_setupinit(task);
    memory_copy(gettaskdirectory(task->id), getkerneldirectory(), sizeof (struct mmu_directory));
    kernel_copyservices(task, task);
    kernel_setupbinary(task, ARCH_TASKSTACKADDRESS);
    kernel_usetask(task);

}

void arch_setup(struct service_backend *backend)
{

    core_init(&core0, 0, ARCH_KERNELSTACKADDRESS + ARCH_KERNELSTACKSIZE);
    arch_configuregdt();
    arch_configureidt();
    arch_configuretss(&tss0, ARCH_TSS, core0.sp);
    mapkernel(0, 0x00000000, 0x00000000, 0x00400000);
    mapkernel(1, 0x00400000, 0x00400000, 0x00400000);
    mapkernel(2, 0x00800000, 0x00800000, 0x00400000);
    mapkernel(3, 0x00C00000, 0x00C00000, 0x00400000);
    mmu_setdirectory(getkerneldirectory());
    mmu_enable();
    kernel_setup();
    kernel_setcallback(coreget, coreassign);
    abi_setup(spawn, despawn);
    binary_setupelf();
    service_setupcpio();
    resource_register(&backend->resource);
    setuptask();
    arch_leave(&core0);

}

