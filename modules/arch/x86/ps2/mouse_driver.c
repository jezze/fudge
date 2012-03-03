#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/modules.h>
#include <modules/ps2/ps2.h>

static void start(struct modules_driver *self)
{

    struct mouse_driver *driver = (struct mouse_driver *)self;

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    return device->type == MOUSE_DEVICE_TYPE;

}

void mouse_driver_init(struct mouse_driver *driver)
{

    modules_driver_init(&driver->base, MOUSE_DRIVER_TYPE, "mouse");

    driver->base.start = start;
    driver->base.check = check;
    driver->cycle = 0;
    driver->x = 0;
    driver->y = 0;

}

