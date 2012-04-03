#include <lib/memory.h>
#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/log.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/ps2/ps2.h>

static void handle_irq(struct modules_device *self)
{

    struct mouse_driver *mouse = (struct mouse_driver *)self->driver;

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

            log_write("Byte 0: 0x%x\n", mouse->status);
            log_write("Byte 1: 0x%x\n", mouse->x);
            log_write("Byte 2: 0x%x\n", mouse->y);

            event_raise(EVENT_IRQ_MOUSE);

            break;

    }

}

static void attach(struct modules_driver *self, struct modules_device *device)
{

    struct mouse_device *mouseDevice = (struct mouse_device *)device;

    irq_register_routine(mouseDevice->irq, device, handle_irq);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    return device->type == MOUSE_DEVICE_TYPE;

}

void mouse_driver_init(struct mouse_driver *driver)
{

    memory_clear(driver, sizeof (struct mouse_driver));

    modules_driver_init(&driver->base, MOUSE_DRIVER_TYPE, "mouse");

    driver->base.attach = attach;
    driver->base.check = check;
    driver->cycle = 2;
    driver->status = 0;
    driver->x = 0;
    driver->y = 0;

}

