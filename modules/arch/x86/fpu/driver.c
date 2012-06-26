#include <lib/memory.h>
#include <kernel/modules.h>
#include <modules/cpuid/cpuid.h>
#include <modules/fpu/fpu.h>
#include <modules/log/log.h>

static void start(struct modules_driver *self)
{

    if (!cpuid_is_supported(CPUID_FEATURES0_EDX_FLAG_FPU))
        return;

    log_write(FPU_DRIVER_TYPE, 13, "Enabling FPU\n");

    fpu_enable();

}

void fpu_driver_init(struct fpu_driver *driver)
{

    memory_clear(driver, sizeof (struct fpu_driver));

    modules_driver_init(&driver->base, FPU_DRIVER_TYPE, "fpu", start, 0, 0);

}

