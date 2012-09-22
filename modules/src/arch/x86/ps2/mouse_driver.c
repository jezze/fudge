#include <memory.h>
#include <base/base.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/io/io.h>
#include <arch/x86/ps2/ps2.h>

static void wait_read()
{

    while ((io_inb(PS2_REGISTER_COMMAND) & 1) != 1);

}

static void wait_write()
{

    while ((io_inb(PS2_REGISTER_COMMAND) & 2) != 0);

}

static void write(unsigned char value)
{

    wait_write();
    io_outb(PS2_REGISTER_COMMAND, 0xD4);
    wait_write();
    io_outb(PS2_REGISTER_DATA, value);

}

static unsigned char read()
{

    wait_read();

    return io_inb(PS2_REGISTER_DATA);

}

static void handle_irq(struct base_device *self)
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

            break;

    }

}

static void start(struct base_driver *self)
{

    unsigned int status;

    wait_write();
    io_outb(PS2_REGISTER_COMMAND, 0xA8);
    wait_write();
    io_outb(PS2_REGISTER_COMMAND, 0x20);
    wait_read();
    status = (io_inb(PS2_REGISTER_DATA) | 2);
    wait_write();
    io_outb(PS2_REGISTER_COMMAND, 0x60);
    wait_write();
    io_outb(PS2_REGISTER_DATA, status);
    write(0xF6);
    read();
    write(0xF4);
    read();

}

static void attach(struct base_device *device)
{

    struct ps2_device *ps2device = (struct ps2_device *)device;

    apic_register_routine(ps2device->irq, device, handle_irq);

}

static unsigned int check(struct base_driver *self, struct base_device *device)
{

    struct ps2_device *ps2device = (struct ps2_device *)device;

    if (device->type != PS2_DEVICE_TYPE)
        return 0;

    return ps2device->irq == PS2_IRQ_MOUSE;

}

void ps2_init_mouse_driver(struct ps2_mouse_driver *driver)
{

    memory_clear(driver, sizeof (struct ps2_mouse_driver));

    base_init_driver(&driver->base, PS2_MOUSE_DRIVER_TYPE, "mouse", start, check, attach);

    driver->cycle = 2;

}

