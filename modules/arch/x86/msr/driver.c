#include <lib/memory.h>
#include <kernel/modules.h>
#include <modules/msr/msr.h>

static void start(struct modules_driver *self)
{

}

void msr_driver_init(struct msr_driver *driver)
{

    memory_clear(driver, sizeof (struct msr_driver));

    modules_driver_init(&driver->base, MSR_DRIVER_TYPE, "msr", start, 0, 0);

    driver->set_msr = msr_set;
    driver->get_msr = msr_get;

}

