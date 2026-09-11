#include <fudge.h>
#include <kernel.h>
#include "udebug.h"
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "isr.h"
#include "mmu.h"
#include "pic.h"
#include "arch.h"

static struct gdt_pointer *gdt = (struct gdt_pointer *)ARCH_GDT_BASE;
static struct idt_pointer *idt = (struct idt_pointer *)ARCH_IDT_BASE;
static struct tss_pointer tss0;
static struct tss_descriptor tssdescriptors0[ARCH_TSS_DESCRIPTORS];
static struct cpu_general registers[POOL_TASKS];

static void map(unsigned long directory, unsigned long mmap, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    struct mmap_header *header = (struct mmap_header *)mmap;

    if (!mmu_gettable(directory, vaddress))
    {

        unsigned long taddress = directory + MMU_PDSIZE + header->ntables * MMU_PTSIZE;

        buffer_clear((void *)taddress, MMU_PTSIZE);
        mmu_settable(directory, vaddress, taddress, mmu_tflags(flags));

        header->ntables++;

    }

    mmu_setpage(directory, vaddress, paddress, mmu_pflags(flags));

}

static void maprange(unsigned long directory, unsigned long mmap, unsigned long vaddress, unsigned long paddress, unsigned int size, unsigned int flags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
        map(directory, mmap, vaddress + i, paddress + i, flags);

}

static void mapentry(unsigned long directory, unsigned long mmap, struct mmap_entry *entry)
{

    switch (entry->type)
    {

    case MMAP_TYPE_NORMAL:
        maprange(directory, mmap, entry->vaddress, entry->paddress, entry->size, entry->flags);

        break;

    case MMAP_TYPE_ZERO:
        maprange(directory, mmap, entry->vaddress, entry->paddress, entry->size, entry->flags);
        buffer_clear((void *)entry->vaddress, entry->size);

        break;

    case MMAP_TYPE_BINARY:
        maprange(directory, mmap, entry->vaddress, entry->paddress, entry->size, entry->flags);

        if (entry->fsize)
            buffer_copy((void *)entry->vaddress, (void *)entry->fbase, entry->fsize);

        if (entry->msize > entry->fsize)
            buffer_clear((void *)(entry->vaddress + entry->fsize), entry->msize - entry->fsize);

        break;

    }

}

static unsigned int createtask(unsigned long address)
{

    unsigned int ntask = pool_picktask();

    if (ntask)
    {

        struct mmap_header *header = (struct mmap_header *)(ARCH_MMAP_BASE + MMAP_SIZE * ntask);
        unsigned int inode = kernel_loadtask(ntask, 0, KERNEL_VSTACK, address, ARCH_MMAP_BASE + MMAP_SIZE * ntask, ARCH_TASK_CODEBASE + TASK_CODESIZE * ntask, ARCH_TASK_STACKBASE + TASK_STACKSIZE * ntask);

        if (inode)
        {

            buffer_copy((void *)(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask), (void *)ARCH_MMU_KERNELBASE, MMU_PDSIZE);
            mapentry(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask, ARCH_MMAP_BASE + MMAP_SIZE * ntask, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMAP_BASE + MMAP_SIZE * ntask, KERNEL_VMMAP, MMAP_SIZE, MMAP_FLAG_WRITEABLE));

            return inode;

        }

    }

    return 0;

}

static unsigned int spawn(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int address;} *args = stack;

    if (args->address)
        return createtask(args->address);

    DEBUG_FMT0(DEBUG_ERROR, "spawn failed");

    return 0;

}

static void schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        buffer_copy(&registers[core->itask], general, sizeof (struct cpu_general));

        task->thread.ip = interrupt->eip.value;
        task->thread.sp = interrupt->esp.value;

    }

    kernel_schedule(core);

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        buffer_copy(general, &registers[core->itask], sizeof (struct cpu_general));

        interrupt->cs.value = gdt_getselector(gdt, ARCH_UCODE);
        interrupt->ss.value = gdt_getselector(gdt, ARCH_UDATA);
        interrupt->eip.value = task->thread.ip;
        interrupt->esp.value = task->thread.sp;

        cpu_setcr3(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * core->itask);

    }

    else
    {

        interrupt->cs.value = gdt_getselector(gdt, ARCH_KCODE);
        interrupt->ss.value = gdt_getselector(gdt, ARCH_KDATA);
        interrupt->eip.value = (unsigned long)cpu_halt;
        interrupt->esp.value = 0;

        cpu_setcr3(ARCH_MMU_KERNELBASE);

    }

}

static void debugpagefault(unsigned int error)
{

    if (error & MMU_EFLAG_PRESENT)
        DEBUG_FMT0(DEBUG_NONE, "Page protection");
    else
        DEBUG_FMT0(DEBUG_NONE, "Non-present page");

    if (error & MMU_EFLAG_RW)
        DEBUG_FMT0(DEBUG_NONE, "Write access violation");
    else
        DEBUG_FMT0(DEBUG_NONE, "Read access violation");

    if (error & MMU_EFLAG_USER)
        DEBUG_FMT0(DEBUG_NONE, "Ring 3");
    else
        DEBUG_FMT0(DEBUG_NONE, "Ring 0");

    if (error & MMU_EFLAG_RESERVED)
        DEBUG_FMT0(DEBUG_NONE, "Reserved");

    if (error & MMU_EFLAG_INSTRUCTION)
        DEBUG_FMT0(DEBUG_NONE, "No-Execute");

    if (error & MMU_EFLAG_PROTECTIONKEY)
        DEBUG_FMT0(DEBUG_NONE, "Protection key");

    if (error & MMU_EFLAG_SHADOWSTACK)
        DEBUG_FMT0(DEBUG_NONE, "Shadow stack");

    if (error & MMU_EFLAG_SGX)
        DEBUG_FMT0(DEBUG_NONE, "SGX violation");

}

static void debugselector(unsigned int error)
{

    unsigned int external = (error & 0x01);
    unsigned int idt = ((error >> 1) & 0x01);
    unsigned int ti = ((error >> 2) & 0x01);
    unsigned int index = ((error >> 3) & 0x1FFF);

    if (external)
        DEBUG_FMT0(DEBUG_NONE, "External");
    else
        DEBUG_FMT0(DEBUG_NONE, "Internal");

    if (ti)
        DEBUG_FMT1(DEBUG_NONE, "LDT: %u", &index);
    else if (idt)
        DEBUG_FMT1(DEBUG_NONE, "IDT: %u", &index);
    else
        DEBUG_FMT1(DEBUG_NONE, "GDT: %u", &index);

}

void arch_kmap(unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int flags)
{

    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate((struct mmap_header *)ARCH_MMAP_BASE, MMAP_TYPE_NORMAL, paddress, vaddress, size, flags));

}

unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    schedule(general, interrupt);

    return interrupt->ss.value;

}

void arch_leave(void)
{

    struct cpu_general general;
    struct cpu_interrupt interrupt;

    buffer_clear(&general, sizeof (struct cpu_general));
    buffer_clear(&interrupt, sizeof (struct cpu_interrupt));

    interrupt.eflags.value = cpu_geteflags() | CPU_FLAGS_IF;

    schedule(&general, &interrupt);
    cpu_leave(interrupt);

}

unsigned short arch_zero(struct cpu_general general, struct cpu_interrupt interrupt)
{

    struct core *core = kernel_getcore();

    DEBUG_FMT0(DEBUG_ERROR, "#DE");

    if (core->itask)
    {

        if (interrupt.cs.value == gdt_getselector(gdt, ARCH_UCODE))
            kernel_signal(core->itask, TASK_SIGNAL_KILL);

    }

    return arch_resume(&general, &interrupt);

}

unsigned short arch_debug(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "#DB");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_nmi(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "Non-maskable interrupt");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_breakpoint(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "#BP");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_overflow(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "#OF");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_bound(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_ERROR, "#BR");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_opcode(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_ERROR, "#UD");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_device(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_ERROR, "#NM");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_doublefault(struct cpu_general general, unsigned int zero, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_ERROR, "#DF %u", &zero);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_tss(struct cpu_general general, unsigned int error, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_ERROR, "#TS %u", &error);
    debugselector(error);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_segment(struct cpu_general general, unsigned int error, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_ERROR, "#NP %u", &error);
    debugselector(error);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_stack(struct cpu_general general, unsigned int error, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_ERROR, "#SS %u", &error);
    debugselector(error);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_generalfault(struct cpu_general general, unsigned int error, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_ERROR, "#GP %u", &error);
    debugselector(error);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_pagefault(struct cpu_general general, unsigned int error, struct cpu_interrupt interrupt)
{

    unsigned int vaddress = cpu_getcr2();
    unsigned int directory = cpu_getcr3();

    if (error & MMU_EFLAG_PRESENT)
    {

        DEBUG_FMT2(DEBUG_CRITICAL, "#PF %u 0x%H8u", &error, &vaddress);
        debugpagefault(error);

        for (;;);

    }

    else
    {

        unsigned int found = 0;

        if (error & MMU_EFLAG_USER)
        {

            struct mmap_entry *entry = mmap_find((struct mmap_header *)KERNEL_VMMAP, vaddress);

            if (entry)
            {

                mapentry(directory, KERNEL_VMMAP, entry);

                found = 1;

            }

        }

        if (!found)
        {

            struct mmap_entry *entry = mmap_find((struct mmap_header *)ARCH_MMAP_BASE, vaddress);

            if (entry)
            {

                mapentry(directory, ARCH_MMAP_BASE, entry);

                found = 1;

            }

        }

        if (!found)
        {

            DEBUG_FMT2(DEBUG_CRITICAL, "#PF %u 0x%H8u", &error, &vaddress);
            debugpagefault(error);

            for (;;);

        }

        if (error & MMU_EFLAG_USER)
        {

        }

        else
        {

            interrupt.cs.value = gdt_getselector(gdt, ARCH_KCODE);
            interrupt.ss.value = gdt_getselector(gdt, ARCH_KDATA);

        }

    }

    return interrupt.ss.value;

}

unsigned short arch_syscall(struct cpu_general general, struct cpu_interrupt interrupt)
{

    struct core *core = kernel_getcore();

    general.eax.value = abi_call(general.eax.value, core->itask, interrupt.esp.reference);

    return arch_resume(&general, &interrupt);

}

void arch_configuregdt(struct gdt_pointer *gdt, struct gdt_descriptor *descriptors, unsigned int count)
{

    gdt_init(gdt, count, descriptors);
    gdt_setdescriptor(gdt, ARCH_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(gdt, ARCH_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(gdt, ARCH_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(gdt, ARCH_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    cpu_setgdt(gdt, gdt_getselector(gdt, ARCH_KCODE), gdt_getselector(gdt, ARCH_KDATA));

}

void arch_configureidt(struct idt_pointer *idt, struct idt_descriptor *descriptors, unsigned int count, unsigned short selector)
{

    idt_init(idt, count, descriptors);
    idt_setdescriptor(idt, 0x00, isr_zero, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x01, isr_debug, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x02, isr_nmi, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x03, isr_breakpoint, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    idt_setdescriptor(idt, 0x04, isr_overflow, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x05, isr_bound, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x06, isr_opcode, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x07, isr_device, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x08, isr_doublefault, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x0A, isr_tss, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x0B, isr_segment, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x0C, isr_stack, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x0D, isr_generalfault, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x0E, isr_pagefault, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0x80, isr_syscall, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    cpu_setidt(idt);

}

void arch_configuretss(struct tss_pointer *tss, struct tss_descriptor *descriptors, unsigned int count, unsigned int id, unsigned short selector1, unsigned short selector2)
{

    tss_init(tss, count, descriptors);
    tss_setdescriptor(tss, 0, selector1, ARCH_KERNEL_STACKBASE + KERNEL_STACKSIZE + KERNEL_STACKSIZE * id);
    gdt_setdescriptor(gdt, ARCH_TSS + id, (unsigned long)descriptors, (unsigned long)descriptors + tss->limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);
    cpu_settss(selector2);

}

static void setupmmap(void)
{

    struct mmap_header *header = (struct mmap_header *)ARCH_MMAP_BASE;

    mmap_initheader(header);
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, 0x00000000, 0x00000000, 0x00100000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_KERNEL_CODEBASE, ARCH_KERNEL_CODEBASE, ARCH_KERNEL_CODESIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_KERNEL_STACKBASE, ARCH_KERNEL_STACKBASE, ARCH_KERNEL_STACKSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMAP_BASE, ARCH_MMAP_BASE, ARCH_MMAP_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMU_KERNELBASE, ARCH_MMU_KERNELBASE, ARCH_MMU_KERNELSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMU_TASKBASE, ARCH_MMU_TASKBASE, ARCH_MMU_TASKSIZE * POOL_TASKS, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MAILBOX_BASE, ARCH_MAILBOX_BASE, ARCH_MAILBOX_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));

}

void arch_setup1(void)
{

    resource_setup();
    udebug_setup();
    pic_init();
    arch_configuregdt(gdt, (struct gdt_descriptor *)(gdt + 1), ARCH_GDT_DESCRIPTORS);
    arch_configureidt(idt, (struct idt_descriptor *)(idt + 1), ARCH_IDT_DESCRIPTORS, gdt_getselector(gdt, ARCH_KCODE));
    arch_configuretss(&tss0, tssdescriptors0, ARCH_TSS_DESCRIPTORS, 0, gdt_getselector(gdt, ARCH_KDATA), gdt_getselector(gdt, ARCH_TSS + 0));
    buffer_clear((void *)ARCH_MMU_KERNELBASE, MMU_PDSIZE);
    cpu_setcr3(ARCH_MMU_KERNELBASE);
    setupmmap();
    mmu_enable();
    mailbox_setup();
    pool_setup(ARCH_MAILBOX_BASE);
    kernel_setup();
    abi_setup();
    abi_setcallback(0x03, spawn);

}

void arch_setup2(unsigned int address)
{

    unsigned int target = createtask(address);

    if (target)
    {

        kernel_place(0, target, EVENT_MAIN, 0, 0);
        arch_leave();

    }

    else
    {

        DEBUG_FMT0(DEBUG_ERROR, "spawn failed");

    }

}

