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
static struct mapping mappings[POOL_TASKS];

static void mapping_clear(struct mapping *mapping)
{

    mmap_initheader((struct mmap_header *)mapping->mmap);
    buffer_clear((void *)mapping->directory, 4096);

}

static void mapping_copy(struct mapping *mapping, struct mapping *from)
{

    mmap_initheader((struct mmap_header *)mapping->mmap);
    buffer_copy((void *)mapping->directory, (void *)from->directory, 4096);

}

static void maptable(unsigned int directory, unsigned int vaddress, unsigned int taddress, unsigned int flags)
{

    unsigned int tflags = MMU_TFLAG_PRESENT;

    if (flags & MMAP_FLAG_USERMODE)
        tflags |= MMU_TFLAG_USERMODE;

    if (flags & MMAP_FLAG_WRITEABLE)
        tflags |= MMU_TFLAG_WRITEABLE;

    if (flags & MMAP_FLAG_WRITETHROUGH)
        tflags |= MMU_TFLAG_WRITETHROUGH;

    mmu_settable(directory, vaddress, taddress | tflags);

}

static void mappage(unsigned int directory, unsigned int vaddress, unsigned int paddress, unsigned int flags)
{

    unsigned int pflags = MMU_PFLAG_PRESENT;

    if (flags & MMAP_FLAG_USERMODE)
        pflags |= MMU_PFLAG_USERMODE;

    if (flags & MMAP_FLAG_WRITEABLE)
        pflags |= MMU_PFLAG_WRITEABLE;

    if (flags & MMAP_FLAG_WRITETHROUGH)
        pflags |= MMU_PFLAG_WRITETHROUGH;

    mmu_setpage(directory, vaddress, paddress | pflags);

}

static unsigned int addtable(struct mmap_header *header, unsigned int directory, unsigned int vaddress)
{

    unsigned int taddress = directory + 4096 + header->ntables * 4096;

    buffer_clear((void *)taddress, 4096);

    header->ntables++;

    return taddress;

}

static void map(unsigned int directory, struct mmap_header *header, unsigned int vaddress, unsigned int paddress, unsigned int flags)
{

    if (!mmu_gettable(directory, vaddress))
    {

        unsigned int taddress = addtable(header, directory, vaddress);

        maptable(directory, vaddress, taddress, flags);

    }

    mappage(directory, vaddress, paddress, flags);

}

static void mapfull(unsigned int directory, struct mmap_header *header, struct mmap_entry *entry)
{

    unsigned int i;

    for (i = 0; i < entry->size; i += MMU_PAGESIZE)
        map(directory, header, entry->vaddress + i, entry->paddress + i, entry->flags);

}

static void mapping_loadcode(struct mapping *mapping, unsigned int address)
{

    struct binary_format *format = binary_findformat(address);

    if (format)
    {

        struct mmap_header *header = (struct mmap_header *)mapping->mmap;
        struct mmap_entry *entry = &header->entries[header->nentries];
        unsigned int offset = 0;
        unsigned int i = 0;

        while ((i = format->mapsection(address, entry, i)))
        {

            if (entry->size)
            {

                entry->paddress = mapping->code + offset;
                offset += (entry->size + MMU_PAGESIZE) & ~MMU_PAGEMASK;
                header->nentries++;
                entry++;

            }

        }

    }

}

static void mapping_loadmmap(struct mapping *mapping)
{

    struct mmap_header *header = (struct mmap_header *)mapping->mmap;
    struct mmap_entry *entry = &header->entries[header->nentries];

    mmap_initentry(entry, MMAP_TYPE_NONE, mapping->mmap, MMAP_VADDRESS, MMAP_SIZE, MMAP_FLAG_WRITEABLE, 0, 0, 0, 0);

    header->nentries++;

    mapfull(mapping->directory, header, entry);

}

static void mapping_loadstack(struct mapping *mapping)
{

    struct mmap_header *header = (struct mmap_header *)mapping->mmap;
    struct mmap_entry *entry = &header->entries[header->nentries];

    mmap_initentry(entry, MMAP_TYPE_NONE, mapping->stack, TASK_STACKVIRTUAL - TASK_STACKSIZE, TASK_STACKSIZE, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE, 0, 0, 0, 0);

    header->nentries++;

}

static unsigned int createtask(unsigned int address)
{

    unsigned int ntask = pool_createtask();

    if (ntask)
    {

        mapping_copy(&mappings[ntask], &mappings[0]);
        mapping_loadcode(&mappings[ntask], address);
        mapping_loadstack(&mappings[ntask]);
        mapping_loadmmap(&mappings[ntask]);

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

        buffer_copy(&mappings[core->itask].registers, general, sizeof (struct cpu_general));

        thread->ip = interrupt->eip.value;
        thread->sp = interrupt->esp.value;

    }

    core->itask = kernel_schedule(core);

    if (core->itask)
    {

        struct task_thread *thread = kernel_gettaskthread(core->itask);

        buffer_copy(general, &mappings[core->itask].registers, sizeof (struct cpu_general));

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_UCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_UDATA);
        interrupt->eip.value = thread->ip;
        interrupt->esp.value = thread->sp;

    }

    else
    {

        interrupt->cs.value = gdt_getselector(&gdt->pointer, ARCH_KCODE);
        interrupt->ss.value = gdt_getselector(&gdt->pointer, ARCH_KDATA);
        interrupt->eip.value = (unsigned int)cpu_halt;
        interrupt->esp.value = core->sp;

    }

    mmu_setdirectory(mappings[core->itask].directory);

}

void arch_kmap(unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int flags)
{

    struct mmap_header *header = (struct mmap_header *)ARCH_MMAPADDRESS;
    struct mmap_entry *entry = &header->entries[header->nentries];

    mmap_initentry(entry, MMAP_TYPE_NONE, paddress, vaddress, size, flags, 0, 0, 0, 0);

    header->nentries++;

    mapfull(mappings[0].directory, header, entry);

}

void arch_umap(unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int flags)
{

    struct mmap_header *header = (struct mmap_header *)MMAP_VADDRESS;
    struct mmap_entry *entry = &header->entries[header->nentries];

    mmap_initentry(entry, MMAP_TYPE_NONE, paddress, vaddress, size, flags, 0, 0, 0, 0);

    header->nentries++;

    mapfull(mmu_getdirectory(), header, entry);

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

unsigned short arch_tss(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_ERROR, "#TS %u", &selector);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_segment(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_ERROR, "#NP %u", &selector);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_stack(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_ERROR, "#SS %u", &selector);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_generalfault(struct cpu_general general, unsigned int selector, struct cpu_interrupt interrupt)
{

    DEBUG_FMT1(DEBUG_ERROR, "#GP %u", &selector);

    return arch_resume(&general, &interrupt);

}

unsigned short arch_pagefault(struct cpu_general general, unsigned int type, struct cpu_interrupt interrupt)
{

    unsigned int vaddress = cpu_getcr2();

    if (type & 0x04)
    {

        struct mmap_header *header = (struct mmap_header *)MMAP_VADDRESS;
        struct mmap_entry *entry = mmap_find(header, vaddress);
        unsigned int directory = mmu_getdirectory();

        DEBUG_FMT2(DEBUG_INFO, "#PF %u 0x%H8u", &type, &vaddress);

        if (entry && entry->size)
        {

            switch (entry->type)
            {

            case MMAP_TYPE_NONE:
                mapfull(directory, header, entry);

                break;

            case MMAP_TYPE_COW:
                mapfull(directory, header, entry);
                buffer_copy((void *)entry->vaddress, (void *)entry->ioaddress, entry->size);

                break;

            case MMAP_TYPE_ZERO:
                mapfull(directory, header, entry);
                buffer_clear((void *)entry->vaddress, entry->size);

                break;

            case MMAP_TYPE_IOCOW:
                mapfull(directory, header, entry);

                if (entry->iofsize)
                    buffer_copy((void *)entry->vaddress, (void *)entry->ioaddress, entry->iofsize);

                if (entry->iomsize > entry->iofsize)
                    buffer_clear((void *)(entry->vaddress + entry->iofsize), entry->iomsize - entry->iofsize);

                break;

            }

        }

        else
        {

            struct mmap_entry *kentry = mmap_find((struct mmap_header *)ARCH_MMAPADDRESS, vaddress);

            if (kentry && kentry->size)
            {

                switch (kentry->type)
                {

                case MMAP_TYPE_NONE:
                    mapfull(directory, header, kentry);

                    break;

                }

            }

        }

    }

    else
    {

        DEBUG_FMT2(DEBUG_CRITICAL, "#PF %u 0x%H8u", &type, &vaddress);

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

static void setupmappings(void)
{

    unsigned int i;

    mappings[0].directory = ARCH_MMUKERNELADDRESS;
    mappings[0].code = ARCH_KERNELCODEADDRESS;
    mappings[0].stack = ARCH_KERNELSTACKADDRESS;
    mappings[0].mmap = ARCH_MMAPADDRESS;

    for (i = 1; i < POOL_TASKS; i++)
    {

        mappings[i].directory = ARCH_MMUTASKADDRESS + ARCH_MMUTASKSIZE * i;
        mappings[i].code = ARCH_TASKCODEADDRESS + (TASK_CODESIZE + TASK_STACKSIZE) * i;
        mappings[i].stack = ARCH_TASKCODEADDRESS + (TASK_CODESIZE + TASK_STACKSIZE) * i + TASK_CODESIZE;
        mappings[i].mmap = ARCH_MMAPADDRESS + MMAP_SIZE * i;

    }

}

void arch_setup1(void)
{

    resource_setup();
    udebug_setup();
    pic_init();
    arch_configuregdt();
    arch_configureidt();
    arch_configuretss(&tss0, 0, ARCH_KERNELSTACKADDRESS + ARCH_KERNELSTACKSIZE);
    setupmappings();
    mapping_clear(&mappings[0]);
    arch_kmap(0x00000000, 0x00000000, 0x00800000, MMAP_FLAG_WRITEABLE);
    arch_kmap(ARCH_MMAPADDRESS, ARCH_MMAPADDRESS, MMAP_SIZE * POOL_TASKS, MMAP_FLAG_WRITEABLE);
    arch_kmap(ARCH_MMUKERNELADDRESS, ARCH_MMUKERNELADDRESS, ARCH_MMUKERNELSIZE, MMAP_FLAG_WRITEABLE);
    arch_kmap(ARCH_MMUTASKADDRESS, ARCH_MMUTASKADDRESS, ARCH_MMUTASKSIZE * POOL_TASKS, MMAP_FLAG_WRITEABLE);
    arch_kmap(ARCH_MAILBOXADDRESS, ARCH_MAILBOXADDRESS, MAILBOX_SIZE * POOL_MAILBOXES, MMAP_FLAG_WRITEABLE);
    arch_kmap(mappings[0].mmap, MMAP_VADDRESS, MMAP_SIZE, MMAP_FLAG_WRITEABLE);
    mmu_setdirectory(mappings[0].directory);
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

