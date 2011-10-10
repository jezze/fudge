#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/io/io.h>
#include <modules/serial/serial.h>

struct serial_device serialDevice1;

static char serial_device_read(struct serial_device *device)
{

    while (!(io_inb(device->port + 5) & 0x01));

    return io_inb(device->port);

}

static void serial_device_write(struct serial_device *device, char c)
{

    while (!(io_inb(device->port + 5) & 0x20));

    io_outb(device->port, c);

}

static unsigned int serial_device_node_read(struct vfs_node *node, unsigned int count, void *buffer)
{

    char *b = (char *)buffer;

    unsigned int i;

    for (i = 0; i < count; i++)
        b[i] = serialDevice1.read(&serialDevice1);

    return i;

}

static unsigned int serial_device_node_write(struct vfs_node *node, unsigned int count, void *buffer)
{

    char *b = (char *)buffer;

    unsigned int i;

    for (i = 0; i < count; i++)
        serialDevice1.write(&serialDevice1, b[i]);

    return i;

}

static void serial_device_init(struct serial_device *device)
{

    io_outb(device->port + 1, 0x00);
    io_outb(device->port + 3, 0x80);
    io_outb(device->port + 0, 0x03);
    io_outb(device->port + 1, 0x00);
    io_outb(device->port + 3, 0x03);
    io_outb(device->port + 2, 0xC7);
    io_outb(device->port + 4, 0x0B);

}

void serial_init()
{

    serialDevice1.base.module.type = MODULES_TYPE_DEVICE;
    serialDevice1.base.type = MODULES_DEVICE_TYPE_SERIAL;
    string_copy(serialDevice1.base.name, "serial");
    serialDevice1.base.node.operations.read = serial_device_node_read;
    serialDevice1.base.node.operations.write = serial_device_node_write;
    serialDevice1.port = SERIAL_COM1;
    serialDevice1.init = serial_device_init;
    serialDevice1.read = serial_device_read;
    serialDevice1.write = serial_device_write;
    modules_register_device(&serialDevice1.base);

    serialDevice1.init(&serialDevice1);

}

