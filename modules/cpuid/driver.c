#include <kernel/modules.h>
#include <modules/cpuid/cpuid.h>

static void is_supported(unsigned int instruction, unsigned int flag)
{

    return cpuid_get_feature_ecx() & flag;

}

static void start(struct modules_driver *self)
{

    struct cpuid_driver *driver = (struct cpuid_driver *)self;

    if (!cpuid_check_cpuid())
        return;

}

void cpuid_driver_init(struct cpuid_driver *driver)
{

    modules_driver_init(&driver->base, CPUID_DRIVER_TYPE);

    driver->base.start = start;
    driver->is_supported = is_supported;

}

