#include <modules.h>
#include <acpi/acpi.h>
#include <cpuid/cpuid.h>
#include <smp/smp.h>

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

