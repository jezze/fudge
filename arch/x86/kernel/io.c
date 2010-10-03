#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/io.h>

struct vfs_node io_node;

unsigned int io_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < count; i++, j++)
        ((char *)buffer)[j] = io_inb(offset);

    return count;

}

unsigned int io_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < count; i++, j++)
        io_outb(offset, ((char *)buffer)[j]);

    return count;

}

void io_init()
{

    memory_set(&io_node, 0, sizeof (struct vfs_node));
    string_copy(io_node.name, "io");
    io_node.read = io_read;
    io_node.write = io_write;

    struct vfs_node *node = call_vfs_find("dev");
    vfs_write(node, node->length, 1, &io_node);

}

