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

static void mapmissing(struct task *task)
{

    struct mmu_directory *kdirectory = getkerneldirectory();
    struct mmu_directory *directory = gettaskdirectory(task->id);
    unsigned int i;

    for (i = 0; i < MMU_TABLES; i++)
    {

        if (kdirectory->tables[i])
            directory->tables[i] = kdirectory->tables[i];

    }

}

static void maptask(struct task *task, unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = gettaskdirectory(task->id);
    struct mmu_table *table = (struct mmu_table *)(directory + 1) + index;

    mmu_map(directory, &table[index], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static unsigned int unloadtask(struct task *task)
{

    kernel_freetask(task);

    return 0;

}

static unsigned int loadtask(struct task *task, unsigned int descriptor)
{

    if (kernel_setupbinary(task, descriptor, ARCH_TASKSTACKADDRESS))
    {

        memory_copy(gettaskdirectory(task->id), getkerneldirectory(), sizeof (struct mmu_directory));
        kernel_usetask(task);
        kernel_reset(task->id);

        return task->id;

    }

    return unloadtask(task);

}

static unsigned int spawn(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct task *next = kernel_picktask();

    if (!next)
        return 0;

    kernel_copydescriptors(task, next);

    return loadtask(next, args->descriptor);

}

static unsigned int despawn(struct task *task, void *stack)
{

    return unloadtask(task);

}

static struct core *coreget(void)
{

    return &core0;

}

static void coreassign(struct task *task)
{

    list_add(&core0.tasks, &task->item);

}

static void schedule(struct cpu_general *general, struct cpu_interrupt *interrupt, struct core *core)
{

    if (core->task)
    {

        memory_copy(&registers[core->task->id], general, sizeof (struct cpu_general));

        core->task->thread.ip = interrupt->eip.value;
        core->task->thread.sp = interrupt->esp.value;

    }

    core_unschedule(core);
    kernel_assign();

    core->task = core_picktask(core);

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

        mmu_setdirectory(getkerneldirectory());

    }

}

void arch_setmap(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = getkerneldirectory();
    struct mmu_table *table = (struct mmu_table *)(directory + 1) + index;

    mmu_map(directory, &table[index], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

void arch_setmapvideo(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = getkerneldirectory();
    struct mmu_table *table = (struct mmu_table *)(directory + 1) + index;

    mmu_map(directory, &table[index], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE | MMU_TFLAG_CACHEWRITE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE | MMU_PFLAG_CACHEWRITE);

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
        unloadtask(core->task);

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
    unsigned int address = cpu_getcr2();
    unsigned int code = core->task->format->findbase(&core->task->node, address);

    DEBUG_LOG(DEBUG_INFO, "exception: page fault");
    mapmissing(core->task);

    if (code)
    {

        maptask(core->task, 0, ARCH_TASKCODEADDRESS + core->task->id * (ARCH_TASKCODESIZE + ARCH_TASKSTACKSIZE), code, ARCH_TASKCODESIZE);
        maptask(core->task, 1, ARCH_TASKCODEADDRESS + core->task->id * (ARCH_TASKCODESIZE + ARCH_TASKSTACKSIZE) + ARCH_TASKCODESIZE, ARCH_TASKSTACKADDRESS - ARCH_TASKSTACKSIZE, ARCH_TASKSTACKSIZE);
        core->task->format->copyprogram(&core->task->node);

    }

    else
    {

        /* TODO: If address is not mapped, task segfaulted and should be killed. */

    }

    return arch_resume(&general, &interrupt);

}

unsigned short arch_syscall(struct cpu_general general, struct cpu_interrupt interrupt)
{

    struct core *core = kernel_getcore();

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
    gdt_setdescriptor(&gdt->pointer, ARCH_TSS + id, (unsigned int)tss->pointer.descriptors, (unsigned int)tss->pointer.descriptors + tss->pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);
    cpu_settss(gdt_getselector(&gdt->pointer, ARCH_TSS + id));

}

void arch_setup1(void)
{

    struct mmu_directory *directory = getkerneldirectory();

    core_init(&core0, 0, ARCH_KERNELSTACKADDRESS + ARCH_KERNELSTACKSIZE, 0);
    arch_configuregdt();
    arch_configureidt();
    arch_configuretss(&tss0, core0.id, core0.sp);
    memory_clear(directory, sizeof (struct mmu_directory));
    arch_setmap(0, 0x00000000, 0x00000000, 0x00400000);
    arch_setmap(1, 0x00400000, 0x00400000, 0x00400000);
    arch_setmap(2, 0x00800000, 0x00800000, 0x00400000);
    arch_setmap(3, 0x00C00000, 0x00C00000, 0x00400000);
    mmu_setdirectory(directory);
    mmu_enable();
    kernel_setup(ARCH_MAILBOXADDRESS, ARCH_MAILBOXSIZE);
    kernel_setcallback(coreget, coreassign);
    abi_setup(spawn, despawn);

}

void arch_setup2(void)
{

    struct task *task = kernel_picktask();

    kernel_setupinit(task);
    kernel_copydescriptors(task, task);
    loadtask(task, 0x04);
    arch_leave(&core0);

}

