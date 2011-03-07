#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/serial/serial.h>

struct modules_serial_device serialDeviceCom1;

static int serial_in_ready(unsigned short port)
{

    return io_inb(port + 5) & 0x01;

}

static char serial_inb(unsigned short port)
{

    while (serial_in_ready(port) == 0);

    return io_inb(port);

}

static unsigned int serial_read(struct modules_serial_device *device, char *buffer, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        buffer[i] = serial_inb(device->port);

    return i;

}

static int serial_out_ready(unsigned short port)
{

    return io_inb(port + 5) & 0x20;

}

static void serial_outb(unsigned short port, char c)
{

    while (serial_out_ready(port) == 0);

    io_outb(port, c);

}

static unsigned int serial_write(struct modules_serial_device *device, char *buffer, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        serial_outb(device->port, buffer[i]);

    return i;

}

void serial_init()
{

    io_outb(SERIAL_COM1 + 1, 0x00);
    io_outb(SERIAL_COM1 + 3, 0x80);
    io_outb(SERIAL_COM1 + 0, 0x03);
    io_outb(SERIAL_COM1 + 1, 0x00);
    io_outb(SERIAL_COM1 + 3, 0x03);
    io_outb(SERIAL_COM1 + 2, 0xC7);
    io_outb(SERIAL_COM1 + 4, 0x0B);

    serialDeviceCom1.port = SERIAL_COM1;
    serialDeviceCom1.read = serial_read;
    serialDeviceCom1.write = serial_write;

    modules_set_serial_device(&serialDeviceCom1);

}

