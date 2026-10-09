#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/udebug.h>
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "isr.h"
#include "mmu.h"
#include <kernel/x86/pic.h>
#include "arch.h"

static struct gdt_pointer *gdt = (struct gdt_pointer *)ARCH_GDT_BASE;
static struct idt_pointer *idt = (struct idt_pointer *)ARCH_IDT_BASE;
static struct tss_pointer *tss = (struct tss_pointer *)ARCH_TSS_BASE;
static struct gdt_descriptor gdtdescriptors[ARCH_GDT_DESCRIPTORS];
static struct idt_descriptor idtdescriptors[ARCH_IDT_DESCRIPTORS];
static struct tss_descriptor tssdescriptors[ARCH_TSS_DESCRIPTORS];
static struct cpu_general registers[POOL_TASKS];

static unsigned long createtable(unsigned long directory, unsigned long mmap)
{

    struct mmap_header *header = (struct mmap_header *)mmap;
    unsigned int size = (directory == ARCH_MMU_KERNELBASE) ? ARCH_MMU_KERNELSIZE : ARCH_MMU_TASKSIZE;
    unsigned long taddress = directory + MMU_TABLESIZE + header->ntables * MMU_TABLESIZE;

    if (taddress + MMU_TABLESIZE > directory + size)
        return 0;

    buffer_clear((void *)taddress, MMU_TABLESIZE);

    header->ntables++;

    return taddress;

}

static unsigned int maptables(unsigned long directory, unsigned long mmap, unsigned long vaddress, unsigned int level, unsigned int flags)
{

    unsigned int i;

    for (i = MMU_LEVELS - 1; i >= level; i--)
    {

        if (!mmu_gettable(directory, vaddress, i))
        {

            unsigned long taddress = createtable(directory, mmap);

            if (!taddress)
                return 0;

            mmu_settable(directory, vaddress, i, taddress, (i > 1) ? MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE : mmu_tflags(flags));

        }

    }

    return 1;

}

static unsigned int map(unsigned long directory, unsigned long mmap, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    if (!maptables(directory, mmap, vaddress, 1, flags))
        return 0;

    mmu_setpage(directory, vaddress, paddress, mmu_pflags(flags));

    return 1;

}

static unsigned int copytable(unsigned long directory, unsigned long mmap, unsigned long vaddress)
{

    unsigned long table = mmu_gettable(ARCH_MMU_KERNELBASE, vaddress, 1);

    if (!(table & MMU_TFLAG_PRESENT) || (mmu_gettable(directory, vaddress, 1) & MMU_TFLAG_PRESENT))
        return 0;

    if (!maptables(directory, mmap, vaddress, 2, 0))
        return 0;

    mmu_settable(directory, vaddress, 1, table, table);

    return 1;

}

static unsigned int maprange(unsigned long directory, unsigned long mmap, unsigned long vaddress, unsigned long paddress, unsigned int size, unsigned int flags)
{

    unsigned long offset = vaddress & (MMU_PAGESIZE - 1);
    unsigned int i;

    for (i = 0; i < offset + size; i += MMU_PAGESIZE)
    {

        if (!map(directory, mmap, vaddress - offset + i, paddress + i, flags))
            return 0;

    }

    return 1;

}

static unsigned int mapentry(unsigned long directory, unsigned long mmap, struct mmap_entry *entry)
{

    if (maprange(directory, mmap, entry->vaddress, entry->paddress, entry->size, entry->flags))
    {

        switch (entry->type)
        {

        case MMAP_TYPE_ZERO:
            buffer_clear((void *)entry->vaddress, entry->size);

            break;

        case MMAP_TYPE_BINARY:
            if (entry->fsize)
                buffer_copy((void *)entry->vaddress, (void *)entry->fbase, entry->fsize);

            if (entry->msize > entry->fsize)
                buffer_clear((void *)(entry->vaddress + entry->fsize), entry->msize - entry->fsize);

            break;

        }

        return 1;

    }

    return 0;

}

static unsigned int createtask(unsigned int pinode, unsigned long address)
{

    unsigned int ntask = pool_picktask();

    if (ntask)
    {

        struct mmap_header *header = (struct mmap_header *)(unsigned long)(ARCH_MMAP_BASE + MMAP_SIZE * ntask);
        unsigned int inode = kernel_loadtask(ntask, 0, KERNEL_VSTACK, pinode, address, ARCH_MMAP_BASE + MMAP_SIZE * ntask, ARCH_TASK_CODEBASE + TASK_CODESIZE * ntask, ARCH_TASK_STACKBASE + TASK_STACKSIZE * ntask);

        if (inode)
        {

            unsigned long vaddress;

            buffer_clear((void *)(unsigned long)(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask), MMU_TABLESIZE);

            for (vaddress = 0; vaddress < ARCH_MMU_LIMIT; vaddress += MMU_PAGESIZE * MMU_ENTRIES)
                copytable(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask, ARCH_MMAP_BASE + MMAP_SIZE * ntask, vaddress);

            mapentry(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask, ARCH_MMAP_BASE + MMAP_SIZE * ntask, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMAP_BASE + MMAP_SIZE * ntask, KERNEL_VMMAP, MMAP_SIZE, MMAP_FLAG_WRITEABLE));
            kernel_starttask(ntask);

            return inode;

        }

        pool_unpicktask(ntask);

    }

    return 0;

}

static unsigned int spawn(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; unsigned int address;} *args = stack;
    unsigned int pinode = kernel_getchannelinode(itask, args->ichannel);

    if (args->address && pinode)
        return createtask(pinode, args->address);

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "spawn failed");

    return 0;

}

static void schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        buffer_copy(&registers[core->itask], general, sizeof (struct cpu_general));

        task->thread.ip = interrupt->rip.value;
        task->thread.sp = interrupt->rsp.value;

    }

    kernel_schedule(core);

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        buffer_copy(general, &registers[core->itask], sizeof (struct cpu_general));

        interrupt->cs.value = gdt_getselector(gdt, ARCH_UCODE);
        interrupt->ss.value = gdt_getselector(gdt, ARCH_UDATA);
        interrupt->rip.value = task->thread.ip;
        interrupt->rsp.value = task->thread.sp;

        cpu_setcr3(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * core->itask);

    }

    else
    {

        interrupt->cs.value = gdt_getselector(gdt, ARCH_KCODE);
        interrupt->ss.value = gdt_getselector(gdt, ARCH_KDATA);
        interrupt->rip.value = (unsigned long)cpu_halt;
        interrupt->rsp.value = (unsigned long)(interrupt + 1);

        cpu_setcr3(ARCH_MMU_KERNELBASE);

    }

}

static void debugpagefault(unsigned long error)
{

    if (error & MMU_EFLAG_PRESENT)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "Page protection");
    else
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "Non-present page");

    if (error & MMU_EFLAG_RW)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "Write access violation");
    else
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "Read access violation");

    if (error & MMU_EFLAG_USER)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "Ring 3");
    else
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "Ring 0");

    if (error & MMU_EFLAG_RESERVED)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "Reserved");

    if (error & MMU_EFLAG_INSTRUCTION)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "No-Execute");

    if (error & MMU_EFLAG_PROTECTIONKEY)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "Protection key");

    if (error & MMU_EFLAG_SHADOWSTACK)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "Shadow stack");

    if (error & MMU_EFLAG_SGX)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "SGX violation");

}

static void debugselector(unsigned long error)
{

    unsigned int external = (error & 0x01);
    unsigned int idt = ((error >> 1) & 0x01);
    unsigned int ti = ((error >> 2) & 0x01);
    unsigned int index = ((error >> 3) & 0x1FFF);

    if (external)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "External");
    else
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "Internal");

    if (ti)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "LDT: %u", &index);
    else if (idt)
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "IDT: %u", &index);
    else
        debug_fmt(DEBUG_NONE, __FILE__, __LINE__, "GDT: %u", &index);

}

void arch_kmap(unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags)
{

    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate((struct mmap_header *)ARCH_MMAP_BASE, MMAP_TYPE_NORMAL, paddress, vaddress, size, flags));

}

unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    schedule(general, interrupt);

    return interrupt->ss.value;

}

static unsigned short fault(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    if (core->itask && interrupt->cs.value == gdt_getselector(gdt, ARCH_UCODE))
    {

        kernel_kill(core->itask, EXIT_STATUS_CRASHED);

        return arch_resume(general, interrupt);

    }

    debug_fmt(DEBUG_CRITICAL, __FILE__, __LINE__, "Kernel fault");

    for (;;);

}

void arch_leave(void)
{

    struct cpu_general general;
    struct cpu_interrupt interrupt;

    buffer_clear(&general, sizeof (struct cpu_general));
    buffer_clear(&interrupt, sizeof (struct cpu_interrupt));

    interrupt.rflags.value = cpu_getrflags() | CPU_FLAGS_IF;

    schedule(&general, &interrupt);
    cpu_leave(&general, &interrupt);

}

unsigned short arch_zero(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "#DE");

    return fault(general, interrupt);

}

unsigned short arch_debug(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "#DB");

    return arch_resume(general, interrupt);

}

unsigned short arch_nmi(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "Non-maskable interrupt");

    return arch_resume(general, interrupt);

}

unsigned short arch_breakpoint(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "#BP");

    return arch_resume(general, interrupt);

}

unsigned short arch_overflow(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "#OF");

    return fault(general, interrupt);

}

unsigned short arch_bound(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "#BR");

    return fault(general, interrupt);

}

unsigned short arch_opcode(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "#UD");

    return fault(general, interrupt);

}

unsigned short arch_device(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "#NM");

    return fault(general, interrupt);

}

unsigned short arch_doublefault(struct cpu_general *general, unsigned long zero, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "#DF %u", &zero);

    return fault(general, interrupt);

}

unsigned short arch_tss(struct cpu_general *general, unsigned long error, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "#TS %u", &error);
    debugselector(error);

    return fault(general, interrupt);

}

unsigned short arch_segment(struct cpu_general *general, unsigned long error, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "#NP %u", &error);
    debugselector(error);

    return fault(general, interrupt);

}

unsigned short arch_stack(struct cpu_general *general, unsigned long error, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "#SS %u", &error);
    debugselector(error);

    return fault(general, interrupt);

}

unsigned short arch_generalfault(struct cpu_general *general, unsigned long error, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "#GP %u", &error);
    debugselector(error);

    return fault(general, interrupt);

}

unsigned short arch_pagefault(struct cpu_general *general, unsigned long error, struct cpu_interrupt *interrupt)
{

    unsigned long vaddress = cpu_getcr2();
    unsigned long directory = cpu_getcr3();
    unsigned int found = 0;

    if (!(error & MMU_EFLAG_PRESENT))
    {

        if (error & MMU_EFLAG_USER)
        {

            struct mmap_entry *entry = mmap_find((struct mmap_header *)KERNEL_VMMAP, vaddress);

            if (entry)
                found = mapentry(directory, KERNEL_VMMAP, entry);

        }

        if (!found)
            found = copytable(directory, KERNEL_VMMAP, vaddress);

    }

    if (!found)
    {

        unsigned int high = vaddress >> 32;
        unsigned int low = vaddress;

        debug_fmt(DEBUG_CRITICAL, __FILE__, __LINE__, "#PF %u 0x%H8u%H8u", &error, &high, &low);
        debugpagefault(error);

        return fault(general, interrupt);

    }

    return interrupt->ss.value;

}

unsigned short arch_syscall(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    general->rax.value = abi_call(general->rax.value, core->itask, interrupt->rsp.reference);

    return arch_resume(general, interrupt);

}

static void configuregdt(void)
{

    gdt_init(gdt, ARCH_GDT_DESCRIPTORS, gdtdescriptors);
    gdt_setdescriptor(gdt, ARCH_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_64BIT);
    gdt_setdescriptor(gdt, ARCH_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(gdt, ARCH_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_64BIT);
    gdt_setdescriptor(gdt, ARCH_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    cpu_setgdt(gdt, gdt_getselector(gdt, ARCH_KCODE), gdt_getselector(gdt, ARCH_KDATA));

}

static void configureidt(void)
{

    unsigned short selector = gdt_getselector(gdt, ARCH_KCODE);

    idt_init(idt, ARCH_IDT_DESCRIPTORS, idtdescriptors);
    idt_setdescriptor(idt, 0x00, isr_zero, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x01, isr_debug, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x02, isr_nmi, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x03, isr_breakpoint, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT | IDT_FLAG_RING3);
    idt_setdescriptor(idt, 0x04, isr_overflow, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x05, isr_bound, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x06, isr_opcode, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x07, isr_device, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x08, isr_doublefault, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x0A, isr_tss, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x0B, isr_segment, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x0C, isr_stack, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x0D, isr_generalfault, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x0E, isr_pagefault, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT);
    idt_setdescriptor(idt, 0x80, isr_syscall, selector, IDT_FLAG_PRESENT | IDT_FLAG_TYPE64INT | IDT_FLAG_RING3);
    cpu_setidt(idt);

}

static void configuretss(void)
{

    unsigned int i;

    tss_init(tss, ARCH_TSS_DESCRIPTORS, tssdescriptors);

    for (i = 0; i < ARCH_TSS_DESCRIPTORS; i++)
    {

        tss_setdescriptor(tss, i, ARCH_KERNEL_STACKBASE + KERNEL_STACKSIZE + KERNEL_STACKSIZE * i);
        gdt_setdescriptor(gdt, ARCH_TSS + i * 2, (unsigned long)&tssdescriptors[i], sizeof (struct tss_descriptor) - 1, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, 0);

    }

    cpu_settss(gdt_getselector(gdt, ARCH_TSS));

}

static void setupmmap(void)
{

    struct mmap_header *header = (struct mmap_header *)ARCH_MMAP_BASE;

    mmap_initheader(header);
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_GDT_BASE, ARCH_GDT_BASE, 0x00001000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_IDT_BASE, ARCH_IDT_BASE, 0x00001000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_TSS_BASE, ARCH_TSS_BASE, 0x00001000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, 0x000B8000, 0x000B8000, 0x00002000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
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
    buffer_clear((void *)ARCH_MMU_KERNELBASE, MMU_TABLESIZE);
    setupmmap();
    mailbox_setup();
    kernel_setup();
    abi_setup();
    abi_setcallback(0x03, spawn);

}

void arch_setup2(void)
{

    cpu_setcr3(ARCH_MMU_KERNELBASE);
    configuregdt();
    configureidt();
    configuretss();
    pic_init();
    pool_setup(ARCH_MAILBOX_BASE);

}

void arch_runinit(unsigned long address)
{

    struct core *core = kernel_getcore();
    unsigned int target = createtask(0, address);

    if (core)
        core_register(core);

    if (target)
    {

        kernel_place(0, target, EVENT_MAIN, 0, 0);
        arch_leave();

    }

    else
    {

        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "spawn failed");

    }

}

