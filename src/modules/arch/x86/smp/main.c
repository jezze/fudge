#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/acpi/acpi.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/apic/apic.h>
#include <modules/arch/x86/ioapic/ioapic.h>
#include "smp.h"

#define INIT16ADDRESS                   0x00008000
#define INIT32ADDRESS                   0x00008100
#define KERNELSTACK                     0x00400000

static struct arch_context current[32];

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

            DEBUG(DEBUG_INFO, "SMP APIC FOUND");

            debug_write(DEBUG_INFO, "  ", "id", apic->id);

        }

        if (entry->type == 1)
        {

            struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;
            unsigned int id;
            unsigned int version;
            unsigned int count;
            unsigned int i;

            DEBUG(DEBUG_INFO, "SMP IOAPIC FOUND");

            arch_setmap(7, ioapic->address, ioapic->address, 0x1000);

            id = ioapic_ind(ioapic->address, 0);
            version = ioapic_ind(ioapic->address, 1);
            count = ((version >> 16) & 0xFF) + 1;

            debug_write(DEBUG_INFO, "  ", "id", ioapic->id);
            debug_write(DEBUG_INFO, "  ", "address", ioapic->address);
            debug_write(DEBUG_INFO, "  ", "id", id);
            debug_write(DEBUG_INFO, "  ", "version", version);

            for (i = 0; i < count; i++)
            {

                unsigned int test;

                test = ioapic_ind(ioapic->address, i * 2 + 0x10);

                debug_write(DEBUG_INFO, "  ", "redtbl0", test);

                test = ioapic_ind(ioapic->address, i * 2 + 0x11);

                debug_write(DEBUG_INFO, "  ", "redtbl1", test);

            }

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

void smp_setup(void)
{

    unsigned int id = 1;

    current[id].task = 0;
    current[id].ip = (unsigned int)cpu_halt;
    current[id].sp = KERNELSTACK - id * 0x8000;

    memory_copy((void *)0xB8000, "X ", 2);

    for (;;);

}

void module_init(void)
{

    copytrampoline16();
    copytrampoline32();
    readmadt();
    apic_sendint(1, APIC_ICR_INIT | APIC_ICR_PHYSICAL | APIC_ICR_ASSERT | APIC_ICR_SINGLE | 0x00); 
    apic_sendint(1, APIC_ICR_SIPI | APIC_ICR_PHYSICAL | APIC_ICR_ASSERT | APIC_ICR_SINGLE | 0x08); 

}

