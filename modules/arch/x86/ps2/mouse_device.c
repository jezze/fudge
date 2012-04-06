#include <lib/memory.h>
#include <kernel/modules.h>
#include <kernel/arch/x86/io.h>
#include <modules/ps2/ps2.h>

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

void ps2_mouse_device_init(struct ps2_mouse_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct ps2_mouse_device));

    modules_device_init(&device->base, PS2_MOUSE_DEVICE_TYPE, "mouse");

    device->irq = irq;

    unsigned char status;

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

