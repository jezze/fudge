#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <kernel/x86/mmu.h>
#include <modules/system/system.h>
#include <modules/arch/x86/acpi/acpi.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/apic/apic.h>
#include <modules/arch/x86/ioapic/ioapic.h>
#include <modules/arch/x86/pit/pit.h>
#include "smp.h"

#define INIT16ADDRESS                   0x00008000
#define INIT32ADDRESS                   0x00008200

struct tss
{

    struct tss_pointer pointer;
    struct tss_descriptor descriptors[ARCH_TSSDESCRIPTORS];

};

static struct tss tss[256];
static struct arch_context context[256];
static unsigned int total;
static struct spinlock spinlock;
static struct system_node root;
static struct system_node cpus;

static struct arch_context *setupcontext(unsigned int id, unsigned int ip, unsigned int sp)
{

    struct arch_context *c = &context[id];

    core_init(&c->core, id);

    c->task = 0;
    c->ip = ip;
    c->sp = sp;

    spinlock_acquire(&spinlock);

    total++;

    spinlock_release(&spinlock);

    return c;

}

static void detect(struct acpi_madt *madt)
{

    unsigned int madttable = (unsigned int)madt + sizeof (struct acpi_madt);
    unsigned int madtend = (unsigned int)madt + madt->base.length;

    while (madttable < madtend)
    {

        struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

        if (entry->type == 0)
        {

            struct acpi_madt_apic *apic = (struct acpi_madt_apic *)entry;

            if (apic->id)
            {

                apic_sendint(apic->id, APIC_ICR_INIT | APIC_ICR_PHYSICAL | APIC_ICR_ASSERT | APIC_ICR_SINGLE | 0x00); 
                pit_wait(10);
                apic_sendint(apic->id, APIC_ICR_SIPI | APIC_ICR_PHYSICAL | APIC_ICR_ASSERT | APIC_ICR_SINGLE | 0x08); 

            }

        }

        if (entry->type == 1)
        {

            struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;
            unsigned int id;
            unsigned int version;
            unsigned int count;

            arch_setmap(7, ioapic->address, ioapic->address, 0x1000);

            id = ioapic_ind(ioapic->address, 0);
            version = ioapic_ind(ioapic->address, 1);
            count = ((version >> 16) & 0xFF) + 1;

            DEBUG(DEBUG_INFO, "SMP IOAPIC");
            debug_write(DEBUG_INFO, "  ", "ioapic id", ioapic->id);
            debug_write(DEBUG_INFO, "  ", "ioapic address", ioapic->address);
            debug_write(DEBUG_INFO, "  ", "ioapic id", id);
            debug_write(DEBUG_INFO, "  ", "ioapic version", version);
            debug_write(DEBUG_INFO, "  ", "ioapic count", count);

        }

        madttable += entry->length;

    }

}

static void copytrampoline16()
{

    unsigned int begin = (unsigned int)smp_begin16;
    unsigned int end = (unsigned int)smp_end16;

    memory_copy((void *)INIT16ADDRESS, (void *)begin, end - begin);

}

static void copytrampoline32()
{

    unsigned int begin = (unsigned int)smp_begin32;
    unsigned int end = (unsigned int)smp_end32;

    memory_copy((void *)INIT32ADDRESS, (void *)begin, end - begin);

}

void smp_setup(unsigned int stack)
{

    unsigned int id = apic_getid();
    struct mmu_directory *directory = (struct mmu_directory *)ARCH_MMUKERNELADDRESS;
    struct arch_context *context = setupcontext(id, (unsigned int)cpu_halt, stack);
    struct gdt_pointer *gpointer = cpu_getgdt();

    tss_initpointer(&tss[id].pointer, ARCH_TSSDESCRIPTORS, tss[id].descriptors);
    tss_setdescriptor(&tss[id].pointer, 0x00, gdt_getselector(gpointer, 2), stack);
    gdt_setdescriptor(gpointer, id + 5, (unsigned int)tss[id].pointer.descriptors, (unsigned int)tss[id].pointer.descriptors + tss[id].pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);
    cpu_settss(gdt_getselector(gpointer, id + 5));
    mmu_setdirectory(directory);
    mmu_enable();
    arch_leave(gdt_getselector(gpointer, 1), gdt_getselector(gpointer, 2), context->ip, context->sp);

}

static unsigned int cpus_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    char num[FUDGE_NSIZE];

    return memory_read(buffer, count, num, ascii_wvalue(num, FUDGE_NSIZE, total, 10, 0), offset);

}

void module_init(void)
{

    unsigned int id = apic_getid();
    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");
    struct arch_context *c = arch_getcontext();

    setupcontext(id, c->ip, c->sp);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "smp");
    system_initnode(&cpus, SYSTEM_NODETYPE_NORMAL, "cpus");

    cpus.read = cpus_read;

    system_addchild(&root, &cpus);

    if (!madt)
        return;

    smp_prep(ARCH_KERNELSTACKADDRESS + 2 * ARCH_KERNELSTACKSIZE);
    copytrampoline16();
    copytrampoline32();
    detect(madt);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

