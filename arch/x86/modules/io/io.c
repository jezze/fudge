#include <lib/call.h>
#include <lib/file.h>
#include <lib/vfs.h>
#include <arch/x86/modules/io/io.h>
#include <kernel/vfs.h>

static unsigned int io_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i = 0;

    for (; count; count--, i++)
        ((char *)buffer)[i] = io_inb(offset);

    return i;

}

static unsigned int io_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i = 0;

    for (; count; count--, i++)
        io_outb(offset, ((char *)buffer)[i]);

    return i;

}

void io_init()
{

    struct vfs_node *ioNode = vfs_add_node("io", 0);
    ioNode->read = io_read;
    ioNode->write = io_write;

    struct vfs_node *devNode = call_open("/dev");
    file_write(devNode, devNode->length, 1, ioNode);

}

