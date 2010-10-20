#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/io.h>

struct vfs_node ioNode;

unsigned int io_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i = 0;

    for (; count; count--, i++)
        ((char *)buffer)[i] = io_inb(offset);

    return i;

}

unsigned int io_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i = 0;

    for (; count; count--, i++)
        io_outb(offset, ((char *)buffer)[i]);

    return i;

}

void io_init()
{

    string_copy(ioNode.name, "io");
    ioNode.read = io_read;
    ioNode.write = io_write;

    struct vfs_node *node = call_open("/dev");
    file_write(node, node->length, 1, &ioNode);

}

