#include <fudge/memory.h>
#include <fudge/string.h>
#include <kernel/vfs.h>
#include <base/base.h>
#include "nodefs.h"

static unsigned int read_root(struct nodefs_filesystem *filesystem, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned char *b = buffer;
    unsigned int c = 0;
    unsigned int i;

    c += memory_read(b + c, count - c, "../\n", 4, offset);
    offset -= (offset > 4) ? 4 : offset;

    for (i = 0; i < filesystem->nodesCount; i++)
    {

        c += memory_read(b + c, count - c, filesystem->nodes[i]->name, string_length(filesystem->nodes[i]->name), offset);
        offset -= (offset > string_length(filesystem->nodes[i]->name)) ? string_length(filesystem->nodes[i]->name) : offset;

        c += memory_read(b + c, count - c, "\n", 1, offset);
        offset -= (offset > 1) ? 1 : offset;

    }

    return c;

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_node *node;
    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    if (offset > 0)
        return 0;

    if (id == 1)
        return read_root(filesystem, offset, count, buffer);

    node = filesystem->nodes[id - 2];

    return node->read(node, offset, count, buffer);

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;
    struct nodefs_node *node = filesystem->nodes[id - 2];

    return node->write(node, offset, count, buffer);

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;
    unsigned int i;

    if (!count)
        return id;

    for (i = 0; i < filesystem->nodesCount; i++)
    {

        if (memory_match(filesystem->nodes[i]->name, path, string_length(filesystem->nodes[i]->name) + 1))
            return i + 2;

    }

    return 0;

}

static void register_node(struct nodefs_filesystem *self, struct nodefs_node *node, char *name, struct base_module *module, unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer))
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (!self->nodes[i])
        {

            node->name = name;
            node->module = module;
            node->read = read;
            node->write = write;

            self->nodes[i] = node;
            self->nodesCount++;

            break;

        }

    }

}

static void unregister_node(struct nodefs_filesystem *self, struct nodefs_node *node)
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (self->nodes[i] == node)
        {

            self->nodes[i] = 0;
            self->nodesCount--;

            break;

        }

    }

}

void nodefs_init_filesystem(struct nodefs_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct nodefs_filesystem));

    vfs_init_interface(&filesystem->base, 1, "nodefs", 0, 0, read, write, walk, 0);

    filesystem->register_node = register_node;
    filesystem->unregister_node = unregister_node;

}

