#include <lib/memory.h>
#include <kernel/modules.h>
#include <modules/fpu/fpu.h>

static void start(struct modules_driver *self)
{

    fpu_enable();

}

void fpu_driver_init(struct fpu_driver *driver)
{

    memory_clear(driver, sizeof (struct fpu_driver));

    modules_driver_init(&driver->base, FPU_DRIVER_TYPE, "fpu", start, 0, 0);

}

