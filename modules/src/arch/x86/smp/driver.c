#include <memory.h>
#include <modules.h>
#include <acpi/acpi.h>
#include <cpuid/cpuid.h>
#include <smp/smp.h>

static void setup_madt(struct smp_driver *driver, struct acpi_driver *acpiDriver)
{

    unsigned int madttable;
    unsigned int madtend;
    struct acpi_madt *madt = (struct acpi_madt *)acpiDriver->find_header(acpiDriver, "APIC");

    if (!madt)
        return;

    madttable = (unsigned int)madt + sizeof (struct acpi_madt);
    madtend = (unsigned int)madt + madt->base.length;

    while (madttable < madtend)
    {

        struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

        if (entry->type == 0)
        {

            struct acpi_madt_lapic *lapic = (struct acpi_madt_lapic *)entry;

            driver->cpus[driver->count].id = lapic->id;
            driver->count++;

        }

        madttable += entry->length;

    }

}

static void setup_srat(struct smp_driver *driver, struct acpi_driver *acpiDriver)
{

    unsigned int srattable;
    unsigned int sratend;
    struct acpi_srat *srat = (struct acpi_srat *)acpiDriver->find_header(acpiDriver, "SRAT");

    if (!srat)
        return;

    srattable = (unsigned int)srat + sizeof (struct acpi_srat);
    sratend = (unsigned int)srat + srat->base.length;

    while (srattable < sratend)
    {

        struct acpi_srat_entry *entry = (struct acpi_srat_entry *)srattable;

        if (entry->type == 0)
        {

        }

        srattable += entry->length;

    }

}
static void start(struct modules_driver *self)
{

    unsigned int i;
    struct smp_driver *driver = (struct smp_driver *)self;

    for (i = 0; i < 32; i++)
    {

        struct smp_cpu *cpu = &driver->cpus[i];

        cpu->id = 0;
        cpu->core = 0;
        cpu->chip = 0;
        cpu->domain = 0;

    }

    setup_madt(driver, driver->acpiDriver);
    setup_srat(driver, driver->acpiDriver);

    for (i = 0; i < driver->count; i++)
    {

    }

}

void smp_driver_init(struct smp_driver *driver, struct acpi_driver *acpiDriver, struct cpuid_driver *cpuidDriver)
{

    memory_clear(driver, sizeof (struct smp_driver));

    modules_driver_init(&driver->base, SMP_DRIVER_TYPE, "smp", start, 0, 0);

    driver->acpiDriver = acpiDriver;
    driver->cpuidDriver = cpuidDriver;

}

