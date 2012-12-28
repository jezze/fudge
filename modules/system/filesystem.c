#include <fudge/memory.h>
#include <fudge/string.h>
#include <kernel/vfs.h>
#include "system.h"
#include "filesystem.h"

static unsigned int (*readers[32])(struct system_filesystem *filesystem, struct system_node *node, unsigned int offset, unsigned int count, void *buffer);
static unsigned int (*writers[32])(struct system_filesystem *filesystem, struct system_node *node, unsigned int offset, unsigned int count, void *buffer);

static unsigned int read_group(struct system_filesystem *filesystem, struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_group *group = (struct system_group *)node;
    struct system_node *current;
    unsigned char *b = buffer;
    unsigned int c = 0;

    for (current = group->children; current; current = current->next)
    {

        unsigned int l = string_length(current->name);

        c += memory_read(b + c, count - c, current->name, l, offset);
        offset -= (offset > l) ? l : offset;

        c += memory_read(b + c, count - c, "/\n", 2, offset);
        offset -= (offset > 2) ? 2 : offset;

    }

    return c;

}

static unsigned int read_root(struct system_filesystem *filesystem, struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_node *current;
    unsigned char *b = buffer;
    unsigned int c = 0;

    for (current = &filesystem->groups->base; current; current = current->next)
    {

        unsigned int l = string_length(current->name);

        c += memory_read(b + c, count - c, current->name, l, offset);
        offset -= (offset > l) ? l : offset;

        c += memory_read(b + c, count - c, "/\n", 2, offset);
        offset -= (offset > 2) ? 2 : offset;

    }

    return c;

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_filesystem *filesystem = (struct system_filesystem *)self;

    if (id > 1)
    {

        struct system_node *node = (struct system_node *)id;

        return readers[node->type](filesystem, node, offset, count, buffer);

    }

    if (id == 1)
        return read_root(filesystem, 0, offset, count, buffer);

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    if (!count)
        return id;

    return 0;

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_filesystem *filesystem = (struct system_filesystem *)self;

    if (id > 1)
    {

        struct system_node *node = (struct system_node *)id;

        return writers[node->type](filesystem, node, offset, count, buffer);

    }

    return 0;

}

void system_init_filesystem(struct system_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct system_filesystem));

    vfs_init_interface(&filesystem->base, 1, "system", 0, 0, read, write, walk, 0);

    readers[SYSTEM_NODE_TYPE_GROUP] = read_group;

}

