#include <lib/call.h>
#include <lib/file.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <arch/x86/modules/io/io.h>

static struct io_device ioDevice;

static unsigned int io_node_read(struct file_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    char *b = (char *)buffer;

    unsigned int i = 0;

    for (; count; count--, i++)
        b[i] = io_inb(offset);

    return i;

}

static unsigned int io_node_write(struct file_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    char *b = (char *)buffer;

    unsigned int i = 0;

    for (; count; count--, i++)
        io_outb(offset, b[i]);

    return i;

}

void io_init()
{

    string_copy(ioDevice.base.node.name, "io");
    ioDevice.base.node.length = 0;
    ioDevice.base.node.read = io_node_read;
    ioDevice.base.node.write = io_node_write;

    struct file_node *devNode = vfs_find(vfs_get_root(), "dev");
    devNode->write(devNode, devNode->length, 1, &ioDevice.base.node);

    modules_register_device(MODULES_DEVICE_TYPE_IO, &ioDevice.base);

}

