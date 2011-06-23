#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/serial/serial.h>

struct serial_device serialDevice1;

static int serial_in_ready(unsigned short port)
{

    return io_inb(port + 5) & 0x01;

}

static char serial_inb(unsigned short port)
{

    while (serial_in_ready(port) == 0);

    return io_inb(port);

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

static unsigned int serial_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    char *b = (char *)buffer;

    unsigned int i;

    for (i = 0; i < count; i++)
        b[i] = serial_inb(serialDevice1.port);

    return i;

}

static unsigned int serial_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    char *b = (char *)buffer;

    unsigned int i;

    for (i = 0; i < count; i++)
        serial_outb(serialDevice1.port, b[i]);

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

    serialDevice1.port = SERIAL_COM1;
    serialDevice1.base.node.read = serial_read;
    serialDevice1.base.node.write = serial_write;

    modules_register_device(MODULES_DEVICE_TYPE_SERIAL, &serialDevice1.base);

}

