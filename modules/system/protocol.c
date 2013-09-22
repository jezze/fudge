#include <fudge/module.h>
#include <kernel/vfs.h>
#include "system.h"
#include "backend.h"
#include "protocol.h"

static unsigned int root(struct vfs_backend *backend)
{

    struct system_header header;

    backend->read(backend, 0, sizeof (struct system_header), &header);

    return header.root;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    struct system_node *node = (struct system_node *)id;

    return (unsigned int)node->parent;

}

static unsigned int match(struct vfs_backend *backend)
{

    struct system_header header;

    backend->read(backend, 0, sizeof (struct system_header), &header);

    return memory_match(header.id, "FUDGE_SYSTEM", 12);

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    struct system_node *node = (struct system_node *)id;

    return node->open(node);

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    struct system_node *node = (struct system_node *)id;

    return node->close(node);

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_node *node = (struct system_node *)id;

    return node->read(node, offset, count, buffer);

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    struct system_node *node = (struct system_node *)id;
    struct system_node *current;

    if (!count)
        return id;

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

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_node *node = (struct system_node *)id;

    return node->write(node, offset, count, buffer);

}

void system_init_protocol(struct vfs_protocol *protocol)
{

    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, walk, 0);

}

