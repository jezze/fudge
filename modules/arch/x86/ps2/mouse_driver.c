#include <lib/memory.h>
#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/log.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/ps2/ps2.h>

static void handle_irq(struct modules_device *self)
{

    struct ps2_mouse_driver *mouse = (struct ps2_mouse_driver *)self->driver;

    switch (mouse->cycle)
    {

        case 0:

            mouse->status = io_inb(0x60);
            mouse->cycle++;

            break;

        case 1:

            mouse->x = io_inb(0x60);
            mouse->cycle++;

            break;

        case 2:

            mouse->y = io_inb(0x60);
            mouse->cycle = 0;

            event_raise(EVENT_IRQ_MOUSE);

            break;

    }

}

static void attach(struct modules_device *device)
{

    struct ps2_mouse_device *mouseDevice = (struct ps2_mouse_device *)device;

    irq_register_routine(mouseDevice->irq, device, handle_irq);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    return device->type == PS2_MOUSE_DEVICE_TYPE;

}

void ps2_mouse_driver_init(struct ps2_mouse_driver *driver)
{

    memory_clear(driver, sizeof (struct ps2_mouse_driver));

    modules_driver_init(&driver->base, PS2_MOUSE_DRIVER_TYPE, "mouse", 0, check, attach);

    driver->cycle = 2;

}

