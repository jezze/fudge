#include <memory.h>
#include <modules.h>
#include <arch/x86/cpuid/cpuid.h>
#include <arch/x86/fpu/fpu.h>
#include <log/log.h>

static void start(struct modules_driver *self)
{

    if (!cpuid_is_supported(CPUID_FEATURES0_EDX_FLAG_FPU))
        return;

    fpu_enable();

    log_write(FPU_DRIVER_TYPE, 11, "FPU Enabled");

}

void fpu_driver_init(struct fpu_driver *driver)
{

    memory_clear(driver, sizeof (struct fpu_driver));

    modules_driver_init(&driver->base, FPU_DRIVER_TYPE, "fpu", start, 0, 0);

}

