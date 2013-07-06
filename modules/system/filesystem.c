#include <fudge/module.h>
#include <kernel/vfs.h>
#include "system.h"
#include "filesystem.h"

static unsigned int backend_read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "FUDGE_SYSTEM", 12, offset);

}

static unsigned int backend_write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int match(struct vfs_backend *backend)
{

    char buffer[16];

    backend->read(backend, 0, 16, buffer);

    return memory_match(buffer, "FUDGE_SYSTEM", 12);

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    return 1;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return 1;

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

        if (current->type == SYSTEM_NODETYPE_GROUP)
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

static unsigned int read_stream(struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_stream *stream = (struct system_stream *)node;

    return stream->read(stream, offset, count, buffer);

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_backend *b = (struct system_backend *)backend;
    struct system_node *node = (struct system_node *)id;

    if (b->readers[node->type])
        return b->readers[node->type](node, offset, count, buffer);

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    struct system_node *node = (struct system_node *)id;
    struct system_node *current;

    if (!count)
        return id;

    if (vfs_isparent(count, path))
        return walk(backend, (unsigned int)node->parent, count - 3, path + 3);

    if (node->type == SYSTEM_NODETYPE_GROUP)
    {

        struct system_group *group = (struct system_group *)node;

        for (current = group->children; current; current = current->sibling)
        {

            unsigned int l = string_length(current->name);

            if (!memory_match(current->name, path, l))
                continue;

            return (current->type == SYSTEM_NODETYPE_GROUP) ? walk(backend, (unsigned int)current, count - l - 1, path + l + 1) : walk(backend, (unsigned int)current, count - l, path + l);

        }

    }

    return 0;

}

static unsigned int write_stream(struct system_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_stream *stream = (struct system_stream *)node;

    return stream->write(stream, offset, count, buffer);

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_backend *b = (struct system_backend *)backend;
    struct system_node *node = (struct system_node *)id;

    if (b->writers[node->type])
        return b->writers[node->type](node, offset, count, buffer);

    return 0;

}

void system_init_backend(struct system_backend *backend)
{

    memory_clear(backend, sizeof (struct system_backend));
    vfs_init_backend(&backend->base, backend_read, backend_write);
    system_init_group(&backend->root, "/");
    backend->root.node.parent = &backend->root.node;

    backend->readers[SYSTEM_NODETYPE_GROUP] = read_group;
    backend->readers[SYSTEM_NODETYPE_STREAM] = read_stream;
    backend->writers[SYSTEM_NODETYPE_STREAM] = write_stream;

}

void system_init_protocol(struct vfs_protocol *protocol, struct system_backend *backend)
{

    vfs_init_protocol(protocol, (unsigned int)&backend->root, match, open, close, read, write, walk, 0);

}

