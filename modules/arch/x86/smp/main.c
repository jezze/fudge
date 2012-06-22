#include <kernel/modules.h>
#include <modules/acpi/acpi.h>
#include <modules/cpuid/cpuid.h>
#include <modules/smp/smp.h>

static struct smp_driver driver;

void init()
{

    smp_driver_init(&driver, 0, 0);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

