#include <lib/memory.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/ps2/ps2.h>

static void wait_read()
{

    unsigned int timeout = 100000;

    while (timeout--)
    {

        if ((io_inb(0x64) & 1) == 1)
            return;

    }

}

static void wait_write()
{

    unsigned int timeout = 100000;

    while (timeout--)
    {

        if ((io_inb(0x64) & 2) == 0)
            return;

    }

}

static void write(unsigned char value)
{

    wait_write();
    io_outb(0x64, 0xD4);
    wait_write();
    io_outb(0x60, value);

}

static unsigned char read()
{

    wait_read();

    return io_inb(0x60);

}

void mouse_device_init(struct mouse_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct mouse_device));

    modules_device_init(&device->base, MOUSE_DEVICE_TYPE, "mouse");

    device->irq = irq;

    unsigned char status;

    wait_write();
    io_outb(0x64, 0xA8);
    wait_write();
    io_outb(0x64, 0x20);
    wait_read();
    status = (io_inb(0x60) | 2);
    wait_write();
    io_outb(0x64, 0x60);
    wait_write();
    io_outb(0x60, status);
    write(0xF6);
    read();
    write(0xF4);
    read();

}

