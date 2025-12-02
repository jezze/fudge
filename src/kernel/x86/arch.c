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

struct mapping
{

    unsigned int directory;
    unsigned int code;
    unsigned int stack;
    unsigned int mmap;
    struct cpu_general registers;

};

static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTADDRESS;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTADDRESS;
static struct arch_tss tss0;
static struct mapping kmapping;
static struct mapping umapping[POOL_TASKS];

static struct mmap_header *getheader(unsigned int mmap)
{

    return (struct mmap_header *)mmap;

}

static struct mmap_entry *getentry(struct mmap_header *header, unsigned int i)
{

    struct mmap_entry *entries = (struct mmap_entry *)(header + 1);

    return &entries[i];

}

static struct mmap_entry *findentry(struct mmap_header *header, unsigned int vaddress)
{

    unsigned int i;

    for (i = 0; i < header->entries; i++)
    {

        struct mmap_entry *entry = getentry(header, i);

        if (vaddress >= entry->vaddress && vaddress < entry->vaddress + entry->size)
            return entry;

    }

    return 0;

}

static void addentry(struct mmap_header *header, struct mmap_entry *from)
{

    struct mmap_entry *entry = getentry(header, header->entries);

    buffer_copy(entry, from, sizeof (struct mmap_entry));

    header->entries++;

}

static void mapping_initkernel(struct mapping *mapping)
{

    mapping->directory = ARCH_MMUKERNELADDRESS;
    mapping->code = ARCH_KERNELCODEADDRESS;
    mapping->stack = ARCH_KERNELSTACKADDRESS;
    mapping->mmap = ARCH_MMAPADDRESS;
 
    mmap_initheader(getheader(mapping->mmap));
    buffer_clear((void *)mapping->directory, 4096);

}

static void mapping_inittask(struct mapping *mapping, unsigned int itask)
{

    mapping->directory = ARCH_MMUTASKADDRESS + ARCH_MMUTASKSIZE * itask;
    mapping->code = ARCH_TASKCODEADDRESS + (TASK_CODESIZE + TASK_STACKSIZE) * itask;
    mapping->stack = ARCH_TASKCODEADDRESS + (TASK_CODESIZE + TASK_STACKSIZE) * itask + TASK_CODESIZE;
    mapping->mmap = ARCH_MMAPADDRESS + MMAP_SIZE * itask;
 
    mmap_initheader(getheader(mapping->mmap));
    buffer_copy((void *)mapping->directory, (void *)kmapping.directory, 4096);

}

static unsigned int addtable(struct mmap_header *header, unsigned int directory, unsigned int vaddress)
{

    unsigned int address = directory + 4096 + header->ntables * 4096;

    buffer_clear((void *)address, 4096);

    header->ntables++;

    return address;

}

static void map(unsigned int directory, struct mmap_header *header, struct mmap_entry *entry)
{

    unsigned int tflags = MMU_TFLAG_PRESENT;
    unsigned int pflags = MMU_PFLAG_PRESENT;
    unsigned int i;

    for (i = 0; i < entry->size; i += MMU_PAGESIZE)
    {

        unsigned int p = entry->paddress + i;
        unsigned int v = entry->vaddress + i;

        if (!mmu_gettable(directory, v))
        {

            unsigned int taddress = addtable(header, directory, v);

            mmu_settableaddress(directory, v, taddress);

        }

        mmu_setpageaddress(directory, v, p);

    }

    if (entry->flags & MMAP_FLAG_USERMODE)
    {

        tflags |= MMU_TFLAG_USERMODE;
        pflags |= MMU_PFLAG_USERMODE;

    }

    if (entry->flags & MMAP_FLAG_WRITEABLE)
    {

        tflags |= MMU_TFLAG_WRITEABLE;
        pflags |= MMU_PFLAG_WRITEABLE;

    }

    if (entry->flags & MMAP_FLAG_WRITETHROUGH)
    {

        tflags |= MMU_TFLAG_WRITETHROUGH;
        pflags |= MMU_PFLAG_WRITETHROUGH;

    }

    mmu_setflagrange(directory, entry->vaddress, entry->size, tflags, pflags);

}

static void mapping_loadcode(struct mapping *mapping, unsigned int address)
{

    struct mmap_header *header = getheader(mapping->mmap);
    struct binary_format *format = binary_findformat(address);

    if (format)
    {

        struct mmap_entry entry;
        unsigned int offset = 0;
        unsigned int i = 0;

        while ((i = format->mapsection(address, &entry, i)))
        {

            if (entry.size)
            {

                entry.paddress = mapping->code + offset;

                addentry(header, &entry);

                offset += (entry.size + MMU_PAGESIZE) & ~MMU_PAGEMASK;

            }

        }

    }

}

static void mapping_loadmmap(struct mapping *mapping)
{

    struct mmap_header *header = getheader(mapping->mmap);
    struct mmap_entry entry;

    mmap_initentry(&entry, MMAP_TYPE_NONE, mapping->mmap, MMAP_VADDRESS, MMAP_SIZE, MMAP_FLAG_WRITEABLE, 0, 0, 0, 0);
    addentry(header, &entry);
    map(mapping->directory, header, &entry);

}

static void mapping_loadstack(struct mapping *mapping)
{

    struct mmap_header *header = getheader(mapping->mmap);
    struct mmap_entry entry;

    mmap_initentry(&entry, MMAP_TYPE_NONE, mapping->stack, TASK_STACKVIRTUAL - TASK_STACKSIZE, TASK_STACKSIZE, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE, 0, 0, 0, 0);
    addentry(header, &entry);

}

static unsigned int createtask(unsigned int address)
{

    unsigned int ntask = pool_createtask();

    if (ntask)
    {

        struct mapping *mapping = &umapping[ntask];

        mapping_inittask(mapping, ntask);
        mapping_loadcode(mapping, address);
        mapping_loadstack(mapping);
        mapping_loadmmap(mapping);

        return kernel_loadtask(ntask, 0, TASK_STACKVIRTUAL, address);

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

        struct task_thread *thread = kernel_gettaskthread(core->itask);
        struct mapping *mapping = &umapping[core->itask];

        buffer_copy(&mapping->registers, general, sizeof (struct cpu_general));

        thread->ip = interrupt->eip.value;
        thread->sp = interrupt->esp.value;

    }

    core->itask = kernel_schedule(core);

    if (core->itask)
    {

        struct task_thread *thread = kernel_gettaskthread(core->itask);
        struct mapping *mapping = &umapping[core->itask];

        buffer_copy(general, &mapping->registers, sizeof (struct cpu_general));

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_UCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_UDATA);
        interrupt->eip.value = thread->ip;
        interrupt->esp.value = thread->sp;

        mmu_setdirectory(mapping->directory);

    }

    else
    {

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_KCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_KDATA);
        interrupt->eip.value = (unsigned int)cpu_halt;
        interrupt->esp.value = core->sp;

        mmu_setdirectory(kmapping.directory);

    }

}

void arch_kmap(unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmap_header *header = getheader(kmapping.mmap);
    struct mmap_entry entry;

    mmap_initentry(&entry, MMAP_TYPE_NONE, paddress, vaddress, size, MMAP_FLAG_WRITEABLE, 0, 0, 0, 0);
    addentry(header, &entry);
    map(kmapping.directory, header, &entry);

}

void arch_umap(unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmap_header *header = getheader(MMAP_VADDRESS);
    struct mmap_entry entry;

    mmap_initentry(&entry, MMAP_TYPE_NONE, paddress, vaddress, size, MMAP_FLAG_WRITEABLE, 0, 0, 0, 0);
    addentry(header, &entry);
    map(mmu_getdirectory(), header, &entry);

}

void arch_umapvideo(unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmap_header *header = getheader(MMAP_VADDRESS);
    struct mmap_entry entry;

    mmap_initentry(&entry, MMAP_TYPE_NONE, paddress, vaddress, size, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH, 0, 0, 0, 0);
    addentry(header, &entry);
    map(mmu_getdirectory(), header, &entry);

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

    struct mmap_header *header = getheader(MMAP_VADDRESS);
    struct mmap_entry *entry = findentry(header, cpu_getcr2());

    if (entry && entry->size)
    {

        map(mmu_getdirectory(), header, entry);

        switch (entry->type)
        {

        case MMAP_TYPE_COW:
            buffer_copy((void *)entry->vaddress, (void *)entry->ioaddress, entry->size);

            break;

        case MMAP_TYPE_ZERO:
            buffer_clear((void *)entry->vaddress, entry->size);

            break;

        case MMAP_TYPE_IOCOW:
            if (entry->iofsize)
                buffer_copy((void *)entry->vaddress, (void *)entry->ioaddress, entry->iofsize);

            if (entry->iomsize > entry->iofsize)
                buffer_clear((void *)(entry->vaddress + entry->iofsize), entry->iomsize - entry->iofsize);

            break;

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
    mapping_initkernel(&kmapping);
    arch_kmap(0x00000000, 0x00000000, 0x00800000);
    arch_kmap(ARCH_MMAPADDRESS, ARCH_MMAPADDRESS, MMAP_SIZE * POOL_TASKS);
    arch_kmap(ARCH_MMUKERNELADDRESS, ARCH_MMUKERNELADDRESS, ARCH_MMUKERNELSIZE);
    arch_kmap(ARCH_MMUTASKADDRESS, ARCH_MMUTASKADDRESS, ARCH_MMUTASKSIZE * POOL_TASKS);
    arch_kmap(ARCH_MAILBOXADDRESS, ARCH_MAILBOXADDRESS, MAILBOX_SIZE * POOL_MAILBOXES);
    arch_kmap(kmapping.mmap, MMAP_VADDRESS, MMAP_SIZE);
    mmu_setdirectory(kmapping.directory);
    mmu_enable();
    mailbox_setup();
    pool_setup(ARCH_MAILBOXADDRESS, MAILBOX_SIZE);
    kernel_setup(ARCH_KERNELSTACKADDRESS, ARCH_KERNELSTACKSIZE);
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

