#include <fudge/module.h>
#include <kernel/vfs.h>
#include "system.h"
#include "filesystem.h"

static unsigned int open(struct vfs_interface *self, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_interface *self, unsigned int id)
{

    return id;

}

static unsigned int read_group(struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_group *group = (struct system_group *)node;
    struct system_node *current;
    unsigned char *b = buffer;
    unsigned int c = 0;

    c += memory_read(b + c, count - c, "../\n", 4, offset);
    offset -= (offset > 4) ? 4 : offset;

    for (current = group->children; current; current = current->sibling)
    {

        unsigned int l = string_length(current->name);

        c += memory_read(b + c, count - c, current->name, l, offset);
        offset -= (offset > l) ? l : offset;

        if (current->type == SYSTEM_NODE_TYPE_GROUP)
        {

            c += memory_read(b + c, count - c, "/\n", 2, offset);
            offset -= (offset > 2) ? 2 : offset;

        }

        else
        {

            c += memory_read(b + c, count - c, "\n", 1, offset);
            offset -= (offset > 1) ? 1 : offset;

        }

    }

    return c;

}

static unsigned int read_integer(struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_integer *integer = (struct system_integer *)node;
    char num[32];

    return memory_read(buffer, count, num, string_write_num(num, 32, *integer->value, 10), offset);

}

static unsigned int read_string(struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_string *string = (struct system_string *)node;

    return memory_read(buffer, count, string->value, string_length(string->value), offset);

}

static unsigned int read_stream(struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_stream *stream = (struct system_stream *)node;

    return stream->read(stream, offset, count, buffer);

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

    struct system_node *node = (struct system_node *)id;
    struct system_node *current;

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
    {

        if (id == self->rootid)
            return 0;

        return walk(self, (unsigned int)node->parent, count - 3, path + 3);

    }

    if (node->type == SYSTEM_NODE_TYPE_GROUP)
    {

        struct system_group *group = (struct system_group *)node;

        for (current = group->children; current; current = current->sibling)
        {

            unsigned int l = string_length(current->name);

            if (!memory_match(current->name, path, l))
                continue;

            return (current->type == SYSTEM_NODE_TYPE_GROUP) ? walk(self, (unsigned int)current, count - l - 1, path + l + 1) : walk(self, (unsigned int)current, count - l, path + l);

        }

    }

    return 0;

}

static unsigned int write_stream(struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_stream *stream = (struct system_stream *)node;

    return stream->write(stream, offset, count, buffer);

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
    vfs_init_interface(&filesystem->base, (unsigned int)&filesystem->root, "system", open, close, read, write, walk, 0);
    system_init_group(&filesystem->root, "/");

    filesystem->readers[SYSTEM_NODE_TYPE_GROUP] = read_group;
    filesystem->readers[SYSTEM_NODE_TYPE_INTEGER] = read_integer;
    filesystem->readers[SYSTEM_NODE_TYPE_STRING] = read_string;
    filesystem->readers[SYSTEM_NODE_TYPE_STREAM] = read_stream;
    filesystem->writers[SYSTEM_NODE_TYPE_STREAM] = write_stream;

}

