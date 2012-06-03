#include <kernel/modules.h>
#include <modules/cpuid/cpuid.h>

static struct cpuid_driver driver;

void init()
{

    cpuid_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

