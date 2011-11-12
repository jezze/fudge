#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/mouse/mouse.h>

static struct mouse_device mouseDevice;

static void mouse_handler()
{

    struct mouse_device *mouse = &mouseDevice;

    switch (mouse->cycle)
    {

        case 0:

            mouse->byte[0] = io_inb(0x60);
            mouse->cycle++;

            break;

        case 1:

            mouse->byte[1] = io_inb(0x60);
            mouse->cycle++;

            break;

        case 2:

            mouse->byte[2] = io_inb(0x60);
            mouse->x = mouse->byte[1];
            mouse->y = mouse->byte[2];
            mouse->cycle = 0;

            break;

    }

    event_handler(EVENT_IRQ_MOUSE);

}

static void mouse_wait(unsigned char type)
{

    unsigned int time_out = 100000;

    if (type == 0)
    {

        while (time_out--)
        {

            if ((io_inb(0x64) & 1) == 1)
                return;

        }

        return;

    }

    else
    {

        while (time_out--)
        {

            if ((io_inb(0x64) & 2) == 0)
                return;

        }

        return;

    }

}

static void mouse_write(unsigned char value)
{

    mouse_wait(1);
    io_outb(0x64, 0xD4);
    mouse_wait(1);
    io_outb(0x60, value);

}

static unsigned char mouse_read()
{

    mouse_wait(0);

    return io_inb(0x60);

}

void mouse_device_init(struct mouse_device *device)
{

    modules_device_init(&device->base, MOUSE_DEVICE_TYPE);
    device->cycle = 0;
    device->x = 0;
    device->y = 0;

    unsigned char status;

    mouse_wait(1);
    io_outb(0x64, 0xA8);
    mouse_wait(1);
    io_outb(0x64, 0x20);
    mouse_wait(0);
    status = (io_inb(0x60) | 2);
    mouse_wait(1);
    io_outb(0x64, 0x60);
    mouse_wait(1);
    io_outb(0x60, status);
    mouse_write(0xF6);
    mouse_read();
    mouse_write(0xF4);
    mouse_read();

}

void init()
{

    mouse_device_init(&mouseDevice);

    kernel_register_irq(0x0C, mouse_handler);

    modules_register_device(&mouseDevice.base);

}

