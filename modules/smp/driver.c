#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/smp/smp.h>

static void start(struct modules_driver *self)
{

    struct smp_driver *driver = (struct smp_driver *)self;

}

void smp_driver_init(struct smp_driver *driver)
{

    modules_driver_init(&driver->base, SMP_DRIVER_TYPE);

    driver->base.start = start;

}

