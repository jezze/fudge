#include <fudge/memory.h>
#include <fudge/string.h>
#include <kernel/vfs.h>
#include "system.h"
#include "filesystem.h"

static unsigned int read_group(struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
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

static unsigned int read_integer(struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_integer *integer = (struct system_integer *)node;
    char num[32];

    string_write_num(num, integer->value, 16);

    return memory_read(buffer, count, num, string_length(num), offset);

}

static unsigned int read_string(struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_string *string = (struct system_string *)node;

    return memory_read(buffer, count, string->value, string_length(string->value), offset);

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_filesystem *filesystem = (struct system_filesystem *)self;
    struct system_node *node = (struct system_node *)id;

    if (filesystem->readers[node->type])
        return filesystem->readers[node->type](node, offset, count, buffer);

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
    struct system_node *node = (struct system_node *)id;

    if (filesystem->writers[node->type])
        return filesystem->writers[node->type](node, offset, count, buffer);

    return 0;

}

void system_init_filesystem(struct system_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct system_filesystem));

    vfs_init_interface(&filesystem->base, (unsigned int)&filesystem->root, "system", 0, 0, read, write, walk, 0);
    system_init_group(&filesystem->root, "/");

    filesystem->readers[SYSTEM_NODE_TYPE_GROUP] = read_group;
    filesystem->readers[SYSTEM_NODE_TYPE_INTEGER] = read_integer;
    filesystem->readers[SYSTEM_NODE_TYPE_STRING] = read_string;

}

