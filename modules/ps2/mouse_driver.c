#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/modules.h>
#include <modules/ps2/ps2.h>

static void mouse_driver_start(struct modules_driver *self)
{

    struct mouse_driver *driver = (struct mouse_driver *)self;

}

static void mouse_driver_attach(struct modules_driver *self, struct modules_device *device)
{

    device->driver = self;

}

static unsigned int mouse_driver_check(struct modules_driver *self, struct modules_device *device)
{

    if (device->type != MOUSE_DEVICE_TYPE)
        return 0;

    return 1;

}

void mouse_driver_init(struct mouse_driver *driver)
{

    modules_driver_init(&driver->base, MOUSE_DRIVER_TYPE);

    driver->base.start = mouse_driver_start;
    driver->base.attach = mouse_driver_attach;
    driver->base.check = mouse_driver_check;
    driver->cycle = 0;
    driver->x = 0;
    driver->y = 0;

}

