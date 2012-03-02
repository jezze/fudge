#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>

static unsigned int filesystem_read(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    if (id > filesystem->count)
        return 0;

    struct nodefs_node *node = filesystem->nodes[id - 1];

    return node->read(node, count, buffer);

}

static unsigned int filesystem_write(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    if (id > filesystem->count)
        return 0;

    struct nodefs_node *node = filesystem->nodes[id - 1];

    return node->write(node, count, buffer);

}

static char *filesystem_get_name(struct vfs_filesystem *self, unsigned int id)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    if (id > filesystem->count)
        return 0;

    return filesystem->nodes[id - 1]->name;

}

static unsigned int filesystem_find(struct vfs_filesystem *self, char *name)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    unsigned int i;

    for (i = 0; i < filesystem->count; i++)
    {

        if (string_find(filesystem->nodes[i]->name, name))
            return i + 1;

    }

    return 0;

}

static unsigned int filesystem_walk(struct vfs_filesystem *self, unsigned int id)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    if (id > filesystem->count - 1)
        return 0;

    return id + 1;

}

static void register_node(struct nodefs_driver *self, struct nodefs_node *node)
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (!self->filesystem.nodes[i])
        {

            self->filesystem.nodes[i] = node;
            self->filesystem.count++;

            break;

        }

    }

}

static void unregister_node(struct nodefs_driver *self, struct nodefs_node *node)
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (self->filesystem.nodes[i] == node)
        {

            self->filesystem.nodes[i] = 0;
            self->filesystem.count--;

            break;

        }

    }

}

void nodefs_filesystem_init(struct nodefs_filesystem *filesystem)
{

    vfs_filesystem_init(&filesystem->base, "module/", 1, 0, 0, filesystem_read, filesystem_write, filesystem_get_name, filesystem_find, filesystem_walk, 0); 
    filesystem->count = 0;

    memory_clear(filesystem->nodes, sizeof (struct nodefs_node *) * 128);

    vfs_register_filesystem(&filesystem->base);

}

void nodefs_driver_init(struct nodefs_driver *driver)
{

    modules_driver_init(&driver->base, NODEFS_DRIVER_TYPE);
    nodefs_filesystem_init(&driver->filesystem);

    driver->register_node = register_node;
    driver->unregister_node = unregister_node;

}

