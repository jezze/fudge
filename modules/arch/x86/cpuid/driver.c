#include <lib/memory.h>
#include <kernel/modules.h>
#include <modules/cpuid/cpuid.h>

static unsigned int is_supported(unsigned int instruction, unsigned int flag)
{

    struct cpuid_data data;

    cpuid_get(instruction, &data);

    return data.edx & flag;

}

static void start(struct modules_driver *self)
{

    if (!cpuid_check_cpuid())
        return;

}

void cpuid_driver_init(struct cpuid_driver *driver)
{

    memory_clear(driver, sizeof (struct cpuid_driver));

    modules_driver_init(&driver->base, CPUID_DRIVER_TYPE, "cpuid", start, 0, 0);

    driver->is_supported = is_supported;

}

