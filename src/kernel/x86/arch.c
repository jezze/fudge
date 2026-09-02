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

static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDT_BASE;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDT_BASE;
static struct arch_tss tss0;
static struct cpu_general registers[POOL_TASKS];

static void map(unsigned long directory, struct mmap_header *header, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    if (!mmu_gettable(directory, vaddress))
    {

        unsigned long taddress = directory + MMU_PDSIZE + header->ntables * MMU_PTSIZE;

        buffer_clear((void *)taddress, MMU_PTSIZE);
        mmu_settable(directory, vaddress, taddress, mmu_tflags(flags));

        header->ntables++;

    }

    mmu_setpage(directory, vaddress, paddress, mmu_pflags(flags));

}

static void maprange(unsigned long directory, struct mmap_header *header, unsigned long vaddress, unsigned long paddress, unsigned int size, unsigned int flags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
        map(directory, header, vaddress + i, paddress + i, flags);

}

static void mapentry(unsigned long directory, struct mmap_header *header, struct mmap_entry *entry)
{

    switch (entry->type)
    {

    case MMAP_TYPE_NORMAL:
        maprange(directory, header, entry->vaddress, entry->paddress, entry->size, entry->flags);

        break;

    case MMAP_TYPE_ZERO:
        maprange(directory, header, entry->vaddress, entry->paddress, entry->size, entry->flags);
        buffer_clear((void *)entry->vaddress, entry->size);

        break;

    case MMAP_TYPE_BINARY:
        maprange(directory, header, entry->vaddress, entry->paddress, entry->size, entry->flags);

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
        unsigned int inode;

        mmap_initheader(header);

        inode = kernel_loadtask(ntask, 0, KERNEL_VSTACK, address, ARCH_MMAP_BASE + MMAP_SIZE * ntask, ARCH_TASK_CODEBASE + (TASK_CODESIZE + TASK_STACKSIZE) * ntask, ARCH_TASK_CODEBASE + (TASK_CODESIZE + TASK_STACKSIZE) * ntask + TASK_CODESIZE);

        if (inode)
        {

            buffer_copy((void *)(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask), (void *)ARCH_MMU_KERNELBASE, MMU_PDSIZE);
            mapentry(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask, header, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMAP_BASE + MMAP_SIZE * ntask, KERNEL_VMMAP, MMAP_SIZE, MMAP_FLAG_WRITEABLE));

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

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_UCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_UDATA);
        interrupt->eip.value = task->thread.ip;
        interrupt->esp.value = task->thread.sp;

        cpu_setcr3(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * core->itask);

    }

    else
    {

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_KCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_KDATA);
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

    struct mmap_header *header = (struct mmap_header *)ARCH_MMAP_BASE;

    mapentry(ARCH_MMU_KERNELBASE, header, mmap_allocate(header, MMAP_TYPE_NORMAL, paddress, vaddress, size, flags));

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

        if (interrupt.cs.value == gdt_getselector(&gdt->pointer, ARCH_UCODE))
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
    unsigned int found = 0;

    if (error & MMU_EFLAG_PRESENT)
    {

        DEBUG_FMT2(DEBUG_CRITICAL, "#PF %u 0x%H8u", &error, &vaddress);
        debugpagefault(error);

        for (;;);

    }

    else
    {

        if (error & MMU_EFLAG_USER)
        {

            struct mmap_header *header = (struct mmap_header *)KERNEL_VMMAP;
            struct mmap_entry *entry = mmap_find(header, vaddress);

            if (entry)
            {

                mapentry(directory, header, entry);

                found = 1;

            }

        }

        if (!found)
        {

            struct mmap_header *header = (struct mmap_header *)ARCH_MMAP_BASE;
            struct mmap_entry *entry = mmap_find(header, vaddress);

            if (entry)
            {

                mapentry(directory, header, entry);

                found = 1;

            }

        }

        if (!found)
        {

            DEBUG_FMT2(DEBUG_CRITICAL, "#PF %u 0x%H8u", &error, &vaddress);
            debugpagefault(error);

        }

        if (error & MMU_EFLAG_USER)
        {

        }

        else
        {

            interrupt.cs.value = gdt_getselector(&gdt->pointer, ARCH_KCODE);
            interrupt.ss.value = gdt_getselector(&gdt->pointer, ARCH_KDATA);

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

void arch_configuregdt(void)
{

    gdt_init(&gdt->pointer, ARCH_GDT_DESCRIPTORS, gdt->descriptors);
    gdt_setdescriptor(&gdt->pointer, ARCH_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, ARCH_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, ARCH_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, ARCH_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    cpu_setgdt(&gdt->pointer, gdt_getselector(&gdt->pointer, ARCH_KCODE), gdt_getselector(&gdt->pointer, ARCH_KDATA));

}

void arch_configureidt(void)
{

    idt_init(&idt->pointer, ARCH_IDT_DESCRIPTORS, idt->descriptors);
    idt_setdescriptor(&idt->pointer, 0x00, isr_zero, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x01, isr_debug, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x02, isr_nmi, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x03, isr_breakpoint, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    idt_setdescriptor(&idt->pointer, 0x04, isr_overflow, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x05, isr_bound, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x06, isr_opcode, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x07, isr_device, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x08, isr_doublefault, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0A, isr_tss, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0B, isr_segment, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0C, isr_stack, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0D, isr_generalfault, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x0E, isr_pagefault, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, 0x80, isr_syscall, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    cpu_setidt(&idt->pointer);

}

void arch_configuretss(struct arch_tss *tss, unsigned int id)
{

    tss_init(&tss->pointer, ARCH_TSS_DESCRIPTORS, tss->descriptors);
    tss_setdescriptor(&tss->pointer, 0, gdt_getselector(&gdt->pointer, ARCH_KDATA), ARCH_KERNEL_STACKBASE + KERNEL_STACKSIZE + KERNEL_STACKSIZE * id);
    gdt_setdescriptor(&gdt->pointer, ARCH_TSS + id, (unsigned long)tss->pointer.descriptors, (unsigned long)tss->pointer.descriptors + tss->pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);
    cpu_settss(gdt_getselector(&gdt->pointer, ARCH_TSS + id));

}

static void setupmmap(void)
{

    struct mmap_header *header = (struct mmap_header *)ARCH_MMAP_BASE;

    mmap_initheader(header);
    mapentry(ARCH_MMU_KERNELBASE, header, mmap_allocate(header, MMAP_TYPE_NORMAL, 0x00000000, 0x00000000, 0x00100000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, header, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_KERNEL_CODEBASE, ARCH_KERNEL_CODEBASE, ARCH_KERNEL_CODESIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, header, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_KERNEL_STACKBASE, ARCH_KERNEL_STACKBASE, ARCH_KERNEL_STACKSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, header, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMAP_BASE, ARCH_MMAP_BASE, ARCH_MMAP_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, header, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMU_KERNELBASE, ARCH_MMU_KERNELBASE, ARCH_MMU_KERNELSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, header, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMU_TASKBASE, ARCH_MMU_TASKBASE, ARCH_MMU_TASKSIZE * POOL_TASKS, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, header, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MAILBOX_BASE, ARCH_MAILBOX_BASE, MESSAGE_CAPACITY * POOL_MAILBOXES, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));

}

void arch_setup1(void)
{

    resource_setup();
    udebug_setup();
    pic_init();
    arch_configuregdt();
    arch_configureidt();
    arch_configuretss(&tss0, 0);
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

