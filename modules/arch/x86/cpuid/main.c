#include <kernel/modules.h>
#include <modules/cpuid/cpuid.h>

static struct cpuid_driver driver;

unsigned int cpuid_is_supported(unsigned int flag)
{

    return driver.is_supported(1, flag);

}

void init()
{

    cpuid_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

