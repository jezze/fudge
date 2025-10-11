#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "isr.h"
#include "mmu.h"
#include "pic.h"
#include "arch.h"

struct mmap
{

    unsigned int directory;
    unsigned int tables;
    unsigned int entries;

};

static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTADDRESS;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTADDRESS;
static struct cpu_general registers[POOL_TASKS];
static struct arch_tss tss0;
static struct mmap kmmap;
static struct mmap ummap[POOL_TASKS];

static struct mmu_directory *mmap_getdirectory(struct mmap *mmap)
{

    return (struct mmu_directory *)mmap->directory;

}

static struct mmu_table *mmap_gettable(struct mmap *mmap, unsigned int vaddress, unsigned int tflags)
{

    struct mmu_directory *directory = mmap_getdirectory(mmap);
    struct mmu_table *table = mmu_getdirectorytable(directory, vaddress);

    if (!table)
    {

        unsigned int address = mmap->tables + sizeof (struct mmu_table) * mmap->entries;

        buffer_clear((void *)address, sizeof (struct mmu_table));
        mmu_setdirectorytable(directory, address, vaddress, tflags);

        mmap->entries++;

    }

    return mmu_getdirectorytable(directory, vaddress);

}

static void map(struct mmap *mmap, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
    {

        struct mmu_table *table = mmap_gettable(mmap, vaddress + i, tflags);

        mmu_settablepage(table, paddress + i, vaddress + i, pflags);

    }

}

static void mmap_init(struct mmap *mmap, unsigned int address)
{

    mmap->directory = address;
    mmap->tables = address + 4096;
    mmap->entries = 0;

    buffer_clear(mmap_getdirectory(mmap), sizeof (struct mmu_directory));

}

static void mmap_inittask(struct mmap *mmap, unsigned int address, unsigned int paddress)
{

    struct binary_format *format = binary_findformat(address);

    if (format)
    {

        struct mmu_directory *directory = mmap_getdirectory(mmap);
        struct mmu_directory *kdirectory = mmap_getdirectory(&kmmap);
        struct binary_section section;
        unsigned int pageoffset = 0;
        unsigned int i;

        buffer_copy(directory, kdirectory, sizeof (struct mmu_directory));

        for (i = 0; format->readsection(address, &section, i); i++)
        {

            if (section.msize)
            {

                /* TODO: Map read-only sections directly to task->address with offset */
                /* TODO: Map writable section as copy on write */
                unsigned int pagestart = section.vaddress & 0xFFFFF000;
                unsigned int pagesize = (section.msize + 0x1000) & 0xFFFFF000;

                map(mmap, paddress + pageoffset, pagestart, pagesize, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

                pageoffset += pagesize;

            }

        }

        map(mmap, paddress + TASK_CODESIZE, TASK_STACKVIRTUAL - TASK_STACKSIZE, TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
        mmu_setdirectory(directory);

        /* TODO: Do this in the page fault handler */
        /* TODO: Only copy sections that are writable */
        for (i = 0; format->readsection(address, &section, i); i++)
        {

            if (section.fsize)
                buffer_copy((void *)section.vaddress, (void *)(address + section.offset), section.fsize);

            if (section.msize > section.fsize)
                buffer_clear((void *)(section.vaddress + section.fsize), section.msize - section.fsize);

        }

    }

}

static unsigned int spawn(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int address;} *args = stack;

    if (args->address)
    {

        unsigned int ntask = pool_createtask();

        if (ntask)
        {

            unsigned int target = kernel_loadtask(ntask, 0, TASK_STACKVIRTUAL, args->address);

            mmap_init(&ummap[ntask], ARCH_MMUTASKADDRESS + ARCH_MMUTASKSIZE * ntask);
            mmap_inittask(&ummap[ntask], args->address, ARCH_TASKCODEADDRESS + ntask * (TASK_CODESIZE + TASK_STACKSIZE));

            return target;

        }

    }

    DEBUG_FMT0(DEBUG_ERROR, "spawn failed");

    return 0;

}

static void schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    if (core->itask)
    {

        struct task_thread *thread = kernel_gettaskthread(core->itask);

        buffer_copy(&registers[core->itask], general, sizeof (struct cpu_general));

        thread->ip = interrupt->eip.value;
        thread->sp = interrupt->esp.value;

    }

    core->itask = kernel_schedule(core);

    if (core->itask)
    {

        struct task_thread *thread = kernel_gettaskthread(core->itask);

        buffer_copy(general, &registers[core->itask], sizeof (struct cpu_general));

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_UCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_UDATA);
        interrupt->eip.value = thread->ip;
        interrupt->esp.value = thread->sp;

        mmu_setdirectory(mmap_getdirectory(&ummap[core->itask]));

    }

    else
    {

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_KCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_KDATA);
        interrupt->eip.value = (unsigned int)cpu_halt;
        interrupt->esp.value = core->sp;

        mmu_setdirectory(mmap_getdirectory(&kmmap));

    }

}

void arch_map(unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    map(&kmmap, paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

void arch_mapuncached(unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    /* Need to fix this workaround */
    struct core *core = kernel_getcore();

    map(&kmmap, paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_CACHEDISABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_CACHEDISABLE);

    if (core->itask)
    {

        map(&ummap[core->itask], paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_CACHEDISABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_CACHEDISABLE);

    }

}

void arch_mapvideo(unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    map(&kmmap, paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE | MMU_TFLAG_WRITETHROUGH, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE | MMU_PFLAG_WRITETHROUGH);

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

    DEBUG_FMT0(DEBUG_INFO, "exception: divide by zero");

    if (core->itask)
    {

        if (interrupt.cs.value == gdt_getselector(&gdt->pointer, ARCH_UCODE))
            kernel_signal(core->itask, TASK_SIGNAL_KILL);

    }

    return arch_resume(&general, &interrupt);

}

unsigned short arch_debug(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "exception: debug");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_nmi(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "exception: non-maskable interrupt");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_breakpoint(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "exception: breakpoint");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_overflow(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "exception: overflow");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_bound(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "exception: bound range exceeded");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_opcode(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "exception: invalid opcode");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_device(struct cpu_general general, struct cpu_interrupt interrupt)
{

    DEBUG_FMT0(DEBUG_INFO, "exception: device unavailable");

    return arch_resume(&general, &interrupt);

}

unsigned short arch_doublefault(struct cpu_general general, unsigned int zero, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_INFO, "exception: double fault %u", &zero);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_tss(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_INFO, "exception: invalid tss %u", &selector);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_segment(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_INFO, "exception: segment not present %u", &selector);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_stack(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_INFO, "exception: stack segment fault %u", &selector);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_generalfault(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_INFO, "exception: general fault %u", &selector);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_pagefault(struct cpu_general general, unsigned int type, struct cpu_interrupt interrupt)
{

    unsigned int address = cpu_getcr2();
    unsigned int index = address >> 22;

    DEBUG_FMT2(DEBUG_INFO, "exception: page fault at 0x%H8u type %u", &address, &type);

    if (type & (MMU_EFLAG_RW | MMU_EFLAG_USER))
    {

        struct mmu_directory *directory = mmu_getdirectory();
        struct mmu_directory *kdirectory = mmap_getdirectory(&kmmap);

        if (!directory->tables[index] && kdirectory->tables[index])
            directory->tables[index] = kdirectory->tables[index];

    }

    return arch_resume(&general, &interrupt);

}

unsigned short arch_syscall(struct cpu_general general, struct cpu_interrupt interrupt)
{

    struct core *core = kernel_getcore();

    general.eax.value = abi_call(general.eax.value, core->itask, interrupt.esp.reference);

    return arch_resume(&general, &interrupt);

}

void arch_configuregdt(void)
{

    gdt_init(&gdt->pointer, ARCH_GDTDESCRIPTORS, gdt->descriptors);
    gdt_setdescriptor(&gdt->pointer, ARCH_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, ARCH_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, ARCH_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_setdescriptor(&gdt->pointer, ARCH_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    cpu_setgdt(&gdt->pointer, gdt_getselector(&gdt->pointer, ARCH_KCODE), gdt_getselector(&gdt->pointer, ARCH_KDATA));

}

void arch_configureidt(void)
{

    idt_init(&idt->pointer, ARCH_IDTDESCRIPTORS, idt->descriptors);
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

void arch_configuretss(struct arch_tss *tss, unsigned int id, unsigned int sp)
{

    tss_init(&tss->pointer, ARCH_TSSDESCRIPTORS, tss->descriptors);
    tss_setdescriptor(&tss->pointer, 0, gdt_getselector(&gdt->pointer, ARCH_KDATA), sp);
    gdt_setdescriptor(&gdt->pointer, ARCH_TSS + id, (unsigned int)tss->pointer.descriptors, (unsigned int)tss->pointer.descriptors + tss->pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);
    cpu_settss(gdt_getselector(&gdt->pointer, ARCH_TSS + id));

}

void arch_setup1(void)
{

    resource_setup();
    pic_init();
    arch_configuregdt();
    arch_configureidt();
    arch_configuretss(&tss0, 0, ARCH_KERNELSTACKADDRESS + ARCH_KERNELSTACKSIZE);
    mmap_init(&kmmap, ARCH_MMUKERNELADDRESS);
    arch_map(0x00000000, 0x00000000, 0x00400000);
    arch_map(0x00400000, 0x00400000, 0x00400000);
    arch_map(ARCH_MMUKERNELADDRESS, ARCH_MMUKERNELADDRESS, 0x00400000);
    arch_map(ARCH_MMUTASKADDRESS, ARCH_MMUTASKADDRESS, ARCH_MMUTASKSIZE * POOL_TASKS);
    arch_map(ARCH_MAILBOXADDRESS, ARCH_MAILBOXADDRESS, ARCH_MAILBOXSIZE * POOL_MAILBOXES);
    mmu_setdirectory(mmap_getdirectory(&kmmap));
    mmu_enable();
    mailbox_setup();
    pool_setup(ARCH_MAILBOXADDRESS, ARCH_MAILBOXSIZE);
    kernel_setup(ARCH_KERNELSTACKADDRESS, ARCH_KERNELSTACKSIZE);
    abi_setup();
    abi_setcallback(0x03, spawn);

}

void arch_setup2(unsigned int address)
{

    unsigned int ntask = pool_createtask();

    if (ntask)
    {

        unsigned int target = kernel_loadtask(ntask, 0, TASK_STACKVIRTUAL, address);
        unsigned int source = kernel_getchannelinode(ntask, 0);

        mmap_init(&ummap[ntask], ARCH_MMUTASKADDRESS + ARCH_MMUTASKSIZE * ntask);
        mmap_inittask(&ummap[ntask], address, ARCH_TASKCODEADDRESS + ntask * (TASK_CODESIZE + TASK_STACKSIZE));
        kernel_place(source, target, EVENT_MAIN, 0, 0);

    }

    else
    {

        DEBUG_FMT0(DEBUG_ERROR, "spawn failed");

    }

    arch_leave();

}

