#include <kernel/modules.h>
#include <modules/acpi/acpi.h>
#include <modules/cpuid/cpuid.h>
#include <modules/smp/smp.h>

struct smp_driver driver;

void init()
{

    struct acpi_driver *acpiDriver = (struct acpi_driver *)modules_get_driver(ACPI_DRIVER_TYPE);

    if (!acpiDriver)
        return;

    struct cpuid_driver *cpuidDriver = (struct cpuid_driver *)modules_get_driver(CPUID_DRIVER_TYPE);

    if (!cpuidDriver)
        return;

    smp_driver_init(&driver, acpiDriver, cpuidDriver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

