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
    unsigned int entries;
    unsigned int code;
    unsigned int stack;
    unsigned int mmap;
    struct mmap_header *mmapheader;
    struct mmap_entry *mmapentries;

};

static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTADDRESS;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTADDRESS;
static struct cpu_general registers[POOL_TASKS];
static struct arch_tss tss0;
static struct mapping kmapping;
static struct mapping umapping[POOL_TASKS];

static void mapping_init(struct mapping *mapping)
{

    mapping->directory = ARCH_MMUKERNELADDRESS;
    mapping->entries = 0;
    mapping->code = ARCH_KERNELCODEADDRESS;
    mapping->stack = ARCH_KERNELSTACKADDRESS;
    mapping->mmap = ARCH_MMAPADDRESS;
    mapping->mmapheader = (struct mmap_header *)mapping->mmap;
    mapping->mmapentries = (struct mmap_entry *)(mapping->mmapheader + 1);
 
    buffer_clear((void *)mapping->directory, 4096);
    mmap_initheader(mapping->mmapheader);

}

static void mapping_inittask(struct mapping *mapping, unsigned int ntask)
{

    mapping->directory = ARCH_MMUTASKADDRESS + ARCH_MMUTASKSIZE * ntask;
    mapping->entries = 0;
    mapping->code = ARCH_TASKCODEADDRESS + ntask * (TASK_CODESIZE + TASK_STACKSIZE);
    mapping->stack = mapping->code + TASK_CODESIZE;
    mapping->mmap = ARCH_MMAPADDRESS + ARCH_MMAPSIZE * ntask;
    mapping->mmapheader = (struct mmap_header *)mapping->mmap;
    mapping->mmapentries = (struct mmap_entry *)(mapping->mmapheader + 1);
 
    buffer_copy((void *)mapping->directory, (void *)kmapping.directory, 4096);
    mmap_initheader(mapping->mmapheader);

}

static void mapping_map(struct mapping *mapping, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
    {

        unsigned int p = paddress + i;
        unsigned int v = vaddress + i;

        if (!mmu_gettable(mapping->directory, v))
        {

            unsigned int address = mapping->directory + 4096 + 4096 * mapping->entries;

            buffer_clear((void *)address, 4096);
            mmu_settableaddress(mapping->directory, v, address);

            mapping->entries++;

        }

        mmu_setpageaddress(mapping->directory, v, p);

    }

}

static void map(struct mapping *mapping, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    mapping_map(mapping, paddress, vaddress, size);
    mmu_setflagrange(mapping->directory, vaddress, size, tflags, pflags);

}

static void mapping_loadcode(struct mapping *mapping, unsigned int address)
{

    struct binary_format *format = binary_findformat(address);

    if (format)
    {

        unsigned int i = 0;

        while ((i = format->mapsection(address, &mapping->mmapentries[mapping->mmapheader->entries], i)))
        {

            struct mmap_entry *entry = &mapping->mmapentries[mapping->mmapheader->entries];

            if (entry->msize)
            {

                entry->address = address + entry->offset;
                entry->vpaddress = entry->vaddress & ~MMU_PAGEMASK;
                entry->vpsize = (entry->msize + MMU_PAGESIZE) & ~MMU_PAGEMASK;
                entry->paddress = mapping->code + mapping->mmapheader->offset;

                mapping_map(mapping, entry->paddress, entry->vpaddress, entry->vpsize);

                if (entry->flags & 0x02)
                    mmu_setflagrange(mapping->directory, entry->vpaddress, entry->vpsize, MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
                else
                    mmu_setflagrange(mapping->directory, entry->vpaddress, entry->vpsize, MMU_TFLAG_USERMODE, MMU_PFLAG_USERMODE);

                mapping->mmapheader->offset += entry->vpsize;
                mapping->mmapheader->entries++;

            }

        }

    }

}

static void mapping_loadmmap(struct mapping *mapping)
{

    map(mapping, mapping->mmap, MMAP_VADDRESS, ARCH_MMAPSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

static void mapping_loadstack(struct mapping *mapping)
{

    map(mapping, mapping->stack, TASK_STACKVIRTUAL - TASK_STACKSIZE, TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static struct mmap_entry *findmmap(unsigned int mmap, unsigned int vaddress)
{

    struct mmap_header *header = (struct mmap_header *)mmap;
    struct mmap_entry *entries = (struct mmap_entry *)(header + 1);
    unsigned int i;

    for (i = 0; i < header->entries; i++)
    {

        struct mmap_entry *entry = &entries[i];

        if (vaddress >= entry->vaddress && vaddress < entry->vaddress + entry->msize)
            return entry;

    }

    return 0;

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

        buffer_copy(&registers[core->itask], general, sizeof (struct cpu_general));

        thread->ip = interrupt->eip.value;
        thread->sp = interrupt->esp.value;

    }

    core->itask = kernel_schedule(core);

    if (core->itask)
    {

        struct task_thread *thread = kernel_gettaskthread(core->itask);
        struct mapping *mapping = &umapping[core->itask];

        buffer_copy(general, &registers[core->itask], sizeof (struct cpu_general));

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

void arch_map(unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    map(&kmapping, paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

void arch_mapuncached(unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct core *core = kernel_getcore();

    map(&kmapping, paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_CACHEDISABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_CACHEDISABLE);

    /* Need to fix this workaround */
    if (core->itask)
    {

        struct mapping *mapping = &umapping[core->itask];

        map(mapping, paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_CACHEDISABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_CACHEDISABLE);

    }

}

void arch_mapvideo(unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    map(&kmapping, paddress, vaddress, size, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE | MMU_TFLAG_WRITETHROUGH, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE | MMU_PFLAG_WRITETHROUGH);

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

            struct mmap_entry *entry = findmmap(MMAP_VADDRESS, address);

            if (entry)
            {

                mmu_addflagrange(mmu_getdirectory(), entry->vpaddress, entry->vpsize, MMU_TFLAG_PRESENT, MMU_PFLAG_PRESENT);

                if (entry->fsize)
                    buffer_copy((void *)entry->vaddress, (void *)entry->address, entry->fsize);

                if (entry->msize > entry->fsize)
                    buffer_clear((void *)(entry->vaddress + entry->fsize), entry->msize - entry->fsize);

            }

            else
            {

                unsigned int kflags = mmu_gettableflags(kmapping.directory, address);

                if (kflags & MMU_TFLAG_PRESENT)
                    mmu_settable(mmu_getdirectory(), address, mmu_gettable(kmapping.directory, address));

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
    mapping_init(&kmapping);
    arch_map(0x00000000, 0x00000000, 0x00800000);
    arch_map(ARCH_MMAPADDRESS, ARCH_MMAPADDRESS, ARCH_MMAPSIZE * POOL_TASKS);
    arch_map(ARCH_MMUKERNELADDRESS, ARCH_MMUKERNELADDRESS, ARCH_MMUKERNELSIZE);
    arch_map(ARCH_MMUTASKADDRESS, ARCH_MMUTASKADDRESS, ARCH_MMUTASKSIZE * POOL_TASKS);
    arch_map(ARCH_MAILBOXADDRESS, ARCH_MAILBOXADDRESS, ARCH_MAILBOXSIZE * POOL_MAILBOXES);
    arch_map(kmapping.mmap, MMAP_VADDRESS, ARCH_MMAPSIZE);
    mmu_setdirectory(kmapping.directory);
    mmu_enable();
    mailbox_setup();
    pool_setup(ARCH_MAILBOXADDRESS, ARCH_MAILBOXSIZE);
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

