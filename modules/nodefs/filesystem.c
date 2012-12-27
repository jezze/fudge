#include <fudge/memory.h>
#include <fudge/string.h>
#include <kernel/vfs.h>
#include <base/base.h>
#include "nodefs.h"
#include "filesystem.h"

static unsigned int read_root(struct nodefs_filesystem *filesystem, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_node *current;
    unsigned char *b = buffer;
    unsigned int c = 0;

    c += memory_read(b + c, count - c, "../\n", 4, offset);
    offset -= (offset > 4) ? 4 : offset;

    for (current = filesystem->nodes; current; current = current->next)
    {

        c += memory_read(b + c, count - c, current->name, string_length(current->name), offset);
        offset -= (offset > string_length(current->name)) ? string_length(current->name) : offset;

        c += memory_read(b + c, count - c, "\n", 1, offset);
        offset -= (offset > 1) ? 1 : offset;

    }

    return c;

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;
    struct nodefs_node *node = (struct nodefs_node *)id;

    if (id == 1)
        return read_root(filesystem, offset, count, buffer);

    return node->read(node, offset, count, buffer);

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_node *node = (struct nodefs_node *)id;

    return node->write(node, offset, count, buffer);

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;
    struct nodefs_node *current;

    if (!count)
        return id;

    for (current = filesystem->nodes; current; current = current->next)
    {

        if (memory_match(current->name, path, string_length(current->name) + 1))
            return (unsigned int)current;

    }

    return 0;

}

void nodefs_init_filesystem(struct nodefs_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct nodefs_filesystem));

    vfs_init_interface(&filesystem->base, 1, "nodefs", 0, 0, read, write, walk, 0);

}

