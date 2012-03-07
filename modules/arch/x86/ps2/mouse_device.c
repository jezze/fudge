#include <lib/memory.h>
#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/ps2/ps2.h>

static void handle_irq(struct modules_device *self)
{

    struct mouse_driver *mouse = (struct mouse_driver *)self->driver;

    if (!mouse)
        return;

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

    event_raise(EVENT_IRQ_MOUSE);

}

static void wait(unsigned char type)
{

    unsigned int timeout = 100000;

    if (type)
    {

        while (timeout--)
        {

            if ((io_inb(0x64) & 2) == 0)
                return;

        }

    }

    else
    {

        while (timeout--)
        {

            if ((io_inb(0x64) & 1) == 1)
                return;

        }

    }

}

static void write(unsigned char value)
{

    wait(1);
    io_outb(0x64, 0xD4);
    wait(1);
    io_outb(0x60, value);

}

static unsigned char read()
{

    wait(0);

    return io_inb(0x60);

}

void mouse_device_init(struct mouse_device *device)
{

    memory_clear(device, sizeof (struct mouse_device));

    modules_device_init(&device->base, MOUSE_DEVICE_TYPE, "mouse");

    unsigned char status;

    wait(1);
    io_outb(0x64, 0xA8);
    wait(1);
    io_outb(0x64, 0x20);
    wait(0);
    status = (io_inb(0x60) | 2);
    wait(1);
    io_outb(0x64, 0x60);
    wait(1);
    io_outb(0x60, status);
    write(0xF6);
    read();
    write(0xF4);
    read();

    irq_register_routine(0x0C, &device->base, handle_irq);

}

