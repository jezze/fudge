#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "isr.h"
#include "mmu.h"
#include "arch.h"

static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTPHYSICAL;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTPHYSICAL;
static struct cpu_general registers[KERNEL_TASKS];
static struct core core0;
static struct arch_tss tss0;

static struct mmu_directory *getkerneldirectory(void)
{

    return (struct mmu_directory *)ARCH_KERNELMMUPHYSICAL;

}

static struct mmu_directory *gettaskdirectory(unsigned int index)
{

    return (struct mmu_directory *)(ARCH_TASKMMUPHYSICAL + index * ARCH_TASKMMUSIZE);

}

static struct mmu_table *gettable(struct mmu_directory *directory, unsigned int index)
{

    return (struct mmu_table *)(directory + 1) + index;

}

static unsigned int spawn(struct task *task, void *stack)
{

    struct task *ntask = kernel_createtask(task);

    if (ntask)
    {

        kernel_setuptask(ntask, ARCH_TASKSTACKVIRTUAL, FILE_CP);
        buffer_copy(gettaskdirectory(ntask->id), getkerneldirectory(), sizeof (struct mmu_directory));

        return ntask->id;

    }

    return 0;

}

static struct core *coreget(void)
{

    return &core0;

}

static void coreassign(struct list_item *item)
{

    list_add(&core0.tasks, item);

}

static void schedule(struct cpu_general *general, struct cpu_interrupt *interrupt, struct core *core)
{

    if (core->task)
    {

        buffer_copy(&registers[core->task->id], general, sizeof (struct cpu_general));

        core->task->thread.ip = interrupt->eip.value;
        core->task->thread.sp = interrupt->esp.value;

    }

    core->task = kernel_schedule(core, core->task);

    if (core->task)
    {

        buffer_copy(general, &registers[core->task->id], sizeof (struct cpu_general));

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
    struct mmu_table *table = gettable(directory, index);

    buffer_clear(table, sizeof (struct mmu_table));
    mmu_map(directory, table, paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

void arch_setmapvideo(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = getkerneldirectory();
    struct mmu_table *table = gettable(directory, index);

    buffer_clear(table, sizeof (struct mmu_table));
    mmu_map(directory, table, paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE | MMU_TFLAG_CACHEWRITE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE | MMU_PFLAG_CACHEWRITE);

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

    if (core->task)
    {

        if (interrupt.cs.value == gdt_getselector(&gdt->pointer, ARCH_UCODE))
            task_signal(core->task, TASK_SIGNAL_KILL);

    }

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

    DEBUG_LOG(DEBUG_INFO, "exception: page fault");

    if (core->task)
    {

        unsigned int code = core->task->format->findbase(&core->task->node, address);
        struct mmu_directory *directory = gettaskdirectory(core->task->id);

        if (code)
        {

            struct mmu_table *ctable = gettable(directory, 0);
            struct mmu_table *stable = gettable(directory, 1);

            buffer_clear(ctable, sizeof (struct mmu_table));
            buffer_clear(stable, sizeof (struct mmu_table));
            mmu_map(directory, ctable, ARCH_TASKCODEPHYSICAL + core->task->id * (ARCH_TASKCODESIZE + ARCH_TASKSTACKSIZE), code, ARCH_TASKCODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
            mmu_map(directory, stable, ARCH_TASKCODEPHYSICAL + core->task->id * (ARCH_TASKCODESIZE + ARCH_TASKSTACKSIZE) + ARCH_TASKCODESIZE, ARCH_TASKSTACKVIRTUAL - ARCH_TASKSTACKSIZE, ARCH_TASKSTACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

            if (!core->task->format->copyprogram(&core->task->node))
                task_signal(core->task, TASK_SIGNAL_KILL);

        }

        else
        {

            struct mmu_directory *kdirectory = getkerneldirectory();
            unsigned int index = address >> 22;

            if (!directory->tables[index] && kdirectory->tables[index])
                directory->tables[index] = kdirectory->tables[index];
            else
                task_signal(core->task, TASK_SIGNAL_KILL);

        }

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

    resource_setup();
    core_init(&core0, 0, ARCH_KERNELSTACKPHYSICAL + ARCH_KERNELSTACKSIZE);
    arch_configuregdt();
    arch_configureidt();
    arch_configuretss(&tss0, core0.id, core0.sp);
    buffer_clear(directory, sizeof (struct mmu_directory));
    arch_setmap(0, 0x00000000, 0x00000000, 0x00400000);
    arch_setmap(1, 0x00400000, 0x00400000, 0x00400000);
    arch_setmap(2, 0x00800000, 0x00800000, 0x00400000);
    arch_setmap(3, 0x00C00000, 0x00C00000, 0x00400000);
    mmu_setdirectory(directory);
    mmu_enable();
    kernel_setup(ARCH_MAILBOXPHYSICAL, ARCH_MAILBOXSIZE);
    kernel_setcallback(coreget, coreassign);
    abi_setup();
    abi_setcallback(0x0B, spawn);

}

void arch_setup2(void)
{

    struct task *ntask = kernel_createtask(0);

    if (ntask)
    {

        struct descriptor *prog = kernel_getdescriptor(ntask, FILE_PP);
        struct descriptor *pwork = kernel_getdescriptor(ntask, FILE_PW);
        struct descriptor *cwork = kernel_getdescriptor(ntask, FILE_CW);

        cwork->service = service_find(6, "initrd");
        cwork->id = cwork->service->root();

        descriptor_copy(pwork, cwork);
        descriptor_copy(prog, pwork);

        prog->id = prog->service->child(prog->id, "bin", 3);
        prog->id = prog->service->child(prog->id, "init", 4);

        kernel_setuptask(ntask, ARCH_TASKSTACKVIRTUAL, FILE_PP);
        buffer_copy(gettaskdirectory(ntask->id), getkerneldirectory(), sizeof (struct mmu_directory));

    }

    arch_leave(&core0);

}

