#include <memory.h>
#include <modules.h>
#include <arch/x86/cpuid/cpuid.h>
#include <arch/x86/msr/msr.h>

static void start(struct modules_driver *self)
{

    struct cpuid_data data;

    cpuid_get(CPUID_INSTRUCTION_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES0_EDX_FLAG_MSR))
        return;

}

void msr_driver_init(struct msr_driver *driver)
{

    memory_clear(driver, sizeof (struct msr_driver));

    modules_driver_init(&driver->base, MSR_DRIVER_TYPE, "msr", start, 0, 0);

    driver->set_msr = msr_set;
    driver->get_msr = msr_get;

}

