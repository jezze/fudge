#include <lib/call.h>
#include <lib/file.h>
#include <lib/vfs.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <arch/x86/modules/io/io.h>

struct modules_io_device ioDevice;

static unsigned int io_device_read(char *buffer, unsigned int count, unsigned int offset)
{

    unsigned int i = 0;

    for (; count; count--, i++)
        buffer[i] = io_inb(offset);

    return i;

}

static unsigned int io_device_write(char *buffer, unsigned int count, unsigned int offset)
{

    unsigned int i = 0;

    for (; count; count--, i++)
        io_outb(offset, buffer[i]);

    return i;

}

static unsigned int io_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    return ioDevice.read((char *)buffer, count, offset);

}

static unsigned int io_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    return ioDevice.write((char *)buffer, count, offset);

}

void io_init()
{

    ioDevice.read = io_device_read;
    ioDevice.write = io_device_write;

    modules_register_module(&ioDevice.base);

    struct vfs_node *ioNode = vfs_add_node("io", 0);
    ioNode->read = io_node_read;
    ioNode->write = io_node_write;

    struct vfs_node *devNode = call_open("/dev");
    file_write(devNode, devNode->length, 1, ioNode);

}

