#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/acpi/acpi.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include "smp.h"

static struct smp_architecture architecture;

void ioapic_outd(unsigned int base, unsigned int index, unsigned int value)
{

    *(unsigned int *)(base) = index;
    *(unsigned int *)(base + 0x10) = value;

}

unsigned int ioapic_ind(unsigned int base, unsigned int index)
{

    *(unsigned int *)(base) = index;

    return *(unsigned int *)(base + 0x10);
}

static void readmadt(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");
    unsigned int madttable;
    unsigned int madtend;

    if (!madt)
        return;

    madttable = (unsigned int)madt + sizeof (struct acpi_madt);
    madtend = (unsigned int)madt + madt->base.length;

    while (madttable < madtend)
    {

        struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

        if (entry->type == 0)
        {

            struct acpi_madt_apic *apic = (struct acpi_madt_apic *)entry;
            struct smp_cpu *cpu = &architecture.cpus[architecture.ncpus];

            cpu->id = apic->id;

            architecture.ncpus++;

        }

        if (entry->type == 1)
        {

            struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;
            struct smp_io *io = &architecture.ios[architecture.nios];
            unsigned int test;

            io->id = ioapic->id;
            io->address = ioapic->address;

            arch_setmap(7, ioapic->address, ioapic->address, 0x1000);

            test = ioapic_ind(ioapic->address, 0);

            debug_write(DEBUG_INFO, "IOAPIC", "id", test);

            test = ioapic_ind(ioapic->address, 1);

            debug_write(DEBUG_INFO, "IOAPIC", "ver", test);

            test = ioapic_ind(ioapic->address, 0x10);

            debug_write(DEBUG_INFO, "IOAPIC", "redtbl", test);

            test = ioapic_ind(ioapic->address, 0x11);

            debug_write(DEBUG_INFO, "IOAPIC", "redtbl", test);

            test = ioapic_ind(ioapic->address, 0x12);

            debug_write(DEBUG_INFO, "IOAPIC", "redtbl", test);

            test = ioapic_ind(ioapic->address, 0x13);

            debug_write(DEBUG_INFO, "IOAPIC", "redtbl", test);

            architecture.nios++;

        }

        madttable += entry->length;

    }

}

void module_init(void)
{

    unsigned int i;

    readmadt();

    for (i = 0; i < architecture.ncpus; i++)
        DEBUG(DEBUG_INFO, "SMP CPU FOUND");

    for (i = 0; i < architecture.nios; i++)
        DEBUG(DEBUG_INFO, "SMP IO FOUND");

}

