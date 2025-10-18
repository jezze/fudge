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

struct mmap_header
{

    unsigned int offset;
    unsigned int entries;

};

struct mmap_entry
{

    unsigned int address;
    unsigned int fsize;
    unsigned int msize;
    unsigned int paddress;
    unsigned int vaddress;
    unsigned int vpaddress;
    unsigned int vpsize;

};

struct mmap
{

    unsigned int directory;
    unsigned int tables;
    unsigned int entries;
    unsigned int mmapdata;

};

static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTADDRESS;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTADDRESS;
static struct cpu_general registers[POOL_TASKS];
static struct arch_tss tss0;
static struct mmap kmmap;
static struct mmap ummap[POOL_TASKS];

static void mmap_allocate(struct mmap *mmap, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
    {

        unsigned int p = paddress + i;
        unsigned int v = vaddress + i;

        if (!mmu_gettable(mmap->directory, v))
        {

            unsigned int address = mmap->tables + 4096 * mmap->entries;

            buffer_clear((void *)address, 4096);
            mmu_settableaddress(mmap->directory, v, address);

            mmap->entries++;

        }

        mmu_setpageaddress(mmap->directory, v, p);

    }

}

static void mmap_setflags(unsigned int directory, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
    {

        unsigned int v = vaddress + i;

        mmu_settableflags(directory, v, tflags);
        mmu_setpageflags(directory, v, pflags);

    }

}

static void mmap_addflags(unsigned int directory, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
    {

        unsigned int v = vaddress + i;

        mmu_settableflags(directory, v, mmu_gettableflags(directory, v) | tflags);
        mmu_setpageflags(directory, v, mmu_getpageflags(directory, v) | pflags);

    }

}

static void map(struct mmap *mmap, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    mmap_allocate(mmap, paddress, vaddress, size);
    mmap_setflags(mmap->directory, vaddress, size, tflags, pflags);

}

static void mmap_initentry(struct mmap_entry *entry, unsigned int address, unsigned int fsize, unsigned int msize, unsigned int paddress, unsigned int vaddress)
{

    entry->address = address;
    entry->fsize = fsize;
    entry->msize = msize;
    entry->paddress = paddress;
    entry->vaddress = vaddress;
    entry->vpaddress = vaddress & 0xFFFFF000;
    entry->vpsize = (msize + 0x1000) & 0xFFFFF000;

}

static void mmap_initheader(struct mmap_header *header)
{

    header->entries = 0;
    header->offset = 0;

}
 
static void mmap_init(struct mmap *mmap, unsigned int mmuaddress, unsigned int mmapdata)
{

    mmap->directory = mmuaddress;
    mmap->tables = mmuaddress + 4096;
    mmap->entries = 0;
    mmap->mmapdata = mmapdata;

    buffer_clear((void *)mmap->directory, 4096);

}

static struct mmap_entry *mmap_find(struct mmap_header *header, struct mmap_entry *entries, unsigned int vaddress)
{

    unsigned int i;

    for (i = 0; i < header->entries; i++)
    {

        struct mmap_entry *entry = &entries[i];

        if (vaddress >= entry->vaddress && vaddress < entry->vaddress + entry->msize)
            return entry;

    }

    return 0;

}

static void mmap_inittask(struct mmap *mmap, unsigned int address, unsigned int paddress)
{

    struct binary_format *format = binary_findformat(address);

    if (format)
    {

        struct mmap_header *header = (struct mmap_header *)mmap->mmapdata;
        struct mmap_entry *entries = (struct mmap_entry *)(header + 1);
        struct binary_section section;
        unsigned int i;

        mmap_initheader(header);
        buffer_copy((void *)mmap->directory, (void *)kmmap.directory, 4096);
        map(mmap, mmap->mmapdata, ARCH_MMAPVADDRESS, ARCH_MMAPSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

        for (i = 0; format->readsection(address, &section, i); i++)
        {

            if (section.msize)
            {

                struct mmap_entry *entry = &entries[header->entries];

                mmap_initentry(entry, address + section.offset, section.fsize, section.msize, paddress + header->offset, section.vaddress);
                map(mmap, entry->paddress, entry->vpaddress, entry->vpsize, MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

                header->offset += entry->vpsize;
                header->entries++;

            }

        }

        map(mmap, paddress + TASK_CODESIZE, TASK_STACKVIRTUAL - TASK_STACKSIZE, TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
        mmu_setdirectory(mmap->directory);

        {

            header = (struct mmap_header *)ARCH_MMAPVADDRESS;
            entries = (struct mmap_entry *)(header + 1);

            for (i = 0; format->readsection(address, &section, i); i++)
            {

                struct mmap_entry *entry = mmap_find(header, entries, section.vaddress);

                if (entry)
                {

                    mmap_addflags(mmap->directory, entry->vpaddress, entry->vpsize, MMU_TFLAG_PRESENT, MMU_PFLAG_PRESENT);

                    if (entry->fsize)
                        buffer_copy((void *)entry->vaddress, (void *)entry->address, entry->fsize);

                    if (entry->msize > entry->fsize)
                        buffer_clear((void *)(entry->vaddress + entry->fsize), entry->msize - entry->fsize);

                }

            }

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

            mmap_init(&ummap[ntask], ARCH_MMUTASKADDRESS + ARCH_MMUTASKSIZE * ntask, ARCH_MMAPADDRESS + ARCH_MMAPSIZE * ntask);
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

        mmu_setdirectory(ummap[core->itask].directory);

    }

    else
    {

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_KCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_KDATA);
        interrupt->eip.value = (unsigned int)cpu_halt;
        interrupt->esp.value = core->sp;

        mmu_setdirectory(kmmap.directory);

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

    DEBUG_FMT2(DEBUG_INFO, "exception: page fault at 0x%H8u type %u", &address, &type);

    if (type & MMU_EFLAG_PRESENT)
    {

    }

    else
    {

        if (type & MMU_EFLAG_USER)
        {

            unsigned int directory = mmu_getdirectory();
            unsigned int flags = mmu_gettableflags(directory, address);

            if (flags & MMU_TFLAG_PRESENT)
            {

            }

            else
            {

                unsigned int kflags = mmu_gettableflags(kmmap.directory, address);

                if (kflags & MMU_TFLAG_PRESENT)
                    mmu_settable(directory, address, mmu_gettable(kmmap.directory, address));

            }

        }

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
    mmap_init(&kmmap, ARCH_MMUKERNELADDRESS, ARCH_MMAPADDRESS);
    arch_map(0x00000000, 0x00000000, 0x00800000);
    arch_map(ARCH_MMAPADDRESS, ARCH_MMAPADDRESS, ARCH_MMAPSIZE * POOL_TASKS);
    arch_map(ARCH_MMUKERNELADDRESS, ARCH_MMUKERNELADDRESS, ARCH_MMUKERNELSIZE);
    arch_map(ARCH_MMUTASKADDRESS, ARCH_MMUTASKADDRESS, ARCH_MMUTASKSIZE * POOL_TASKS);
    arch_map(ARCH_MAILBOXADDRESS, ARCH_MAILBOXADDRESS, ARCH_MAILBOXSIZE * POOL_MAILBOXES);
    arch_map(kmmap.mmapdata, ARCH_MMAPVADDRESS, ARCH_MMAPSIZE);
    mmu_setdirectory(kmmap.directory);
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

        mmap_init(&ummap[ntask], ARCH_MMUTASKADDRESS + ARCH_MMUTASKSIZE * ntask, ARCH_MMAPADDRESS + ARCH_MMAPSIZE * ntask);
        mmap_inittask(&ummap[ntask], address, ARCH_TASKCODEADDRESS + ntask * (TASK_CODESIZE + TASK_STACKSIZE));
        kernel_place(source, target, EVENT_MAIN, 0, 0);

    }

    else
    {

        DEBUG_FMT0(DEBUG_ERROR, "spawn failed");

    }

    arch_leave();

}

