#include <lib/memory.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/acpi/acpi.h>
#include <modules/cpuid/cpuid.h>
#include <modules/smp/smp.h>

static void setup_madt(struct smp_driver *driver, struct acpi_driver *acpiDriver)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpiDriver->find_header(acpiDriver, "APIC");

    if (!madt)
        return;

    log_write("[smp] Madt found\n");

    void *madttable = (void *)madt + sizeof (struct acpi_madt);
    void *madtend = (void *)madt + madt->base.length;

    while (madttable < madtend)
    {

        struct acpi_madt_entry *entry = madttable;

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

    struct acpi_srat *srat = (struct acpi_srat *)acpiDriver->find_header(acpiDriver, "SRAT");

    if (!srat)
        return;

    log_write("[smp] Srat found\n");

    void *srattable = (void *)srat + sizeof (struct acpi_srat);
    void *sratend = (void *)srat + srat->base.length;

    while (srattable < sratend)
    {

        struct acpi_srat_entry *entry = srattable;
        if (entry->type == 0)
        {

        }

        srattable += entry->length;

    }

}
static void start(struct modules_driver *self)
{

    struct smp_driver *driver = (struct smp_driver *)self;

//    if (!driver->cpuidDriver->is_supported(CPUID_INSTRUCTION_VENDOR, CPUID_FEATURES0_EDX_FLAG_HTT))
//        return;

    unsigned int i;

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

        struct smp_cpu *cpu = &driver->cpus[i];

        log_write("[smp] CPU %d: %d:%d:%d:%d\n", i, cpu->domain, cpu->chip, cpu->core, cpu->id);

    }

}

void smp_driver_init(struct smp_driver *driver, struct acpi_driver *acpiDriver, struct cpuid_driver *cpuidDriver)
{

    memory_clear(driver, sizeof (struct smp_driver));

    modules_driver_init(&driver->base, SMP_DRIVER_TYPE, "smp", start, 0, 0);

    driver->acpiDriver = acpiDriver;
    driver->cpuidDriver = cpuidDriver;

}

