#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/acpi/acpi.h>
#include <modules/cpuid/cpuid.h>
#include <modules/smp/smp.h>

static void start(struct modules_driver *self)
{

    struct smp_driver *driver = (struct smp_driver *)self;

    struct acpi_driver *acpiDriver = (struct acpi_driver *)modules_get_driver(ACPI_DRIVER_TYPE);

    if (!acpiDriver)
        return;

    struct cpuid_driver *cpuidDriver = (struct cpuid_driver *)modules_get_driver(CPUID_DRIVER_TYPE);

    if (!cpuidDriver)
        return;

    log_write("[smp] Driver started\n");

}

void smp_driver_init(struct smp_driver *driver)
{

    modules_driver_init(&driver->base, SMP_DRIVER_TYPE);

    driver->base.start = start;

}

