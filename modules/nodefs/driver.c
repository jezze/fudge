#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <modules/nodefs/nodefs.h>

static unsigned int filesystem_read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    if (id == 1)
    {

        char *offset = buffer;
        unsigned int i;

        for (i = 0; i < filesystem->count; i++)
        {

            unsigned int length = string_length(filesystem->nodes[i]->name);

            memory_copy(offset, filesystem->nodes[i]->name, length);
            memory_copy(offset + length, "\n", 1);
            offset += length + 1;

        }

        return string_length(buffer);

    }

    struct nodefs_node *node = filesystem->nodes[id - 2];

    return node->read(node, offset, count, buffer);

}

static unsigned int filesystem_write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    struct nodefs_node *node = filesystem->nodes[id - 2];

    return node->write(node, offset, count, buffer);

}

static unsigned int filesystem_find(struct modules_filesystem *self, char *name)
{

    unsigned int length = string_length(name);

    if (!length)
        return 1;

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    unsigned int i;

    for (i = 0; i < filesystem->count; i++)
    {

        if (string_find(filesystem->nodes[i]->name, name))
            return i + 2;

    }

    return 0;

}

static void register_node(struct nodefs_driver *self, struct nodefs_node *node, char *name, struct modules_base *module, unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer))
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (!self->filesystem.nodes[i])
        {

            node->name = name;
            node->module = module;
            node->read = read;
            node->write = write;

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

    memory_clear(filesystem, sizeof (struct nodefs_filesystem));

    modules_filesystem_init(&filesystem->base, 0x0001, "nodefs", 0, 0, filesystem_read, filesystem_write, filesystem_find, 0); 
    filesystem->count = 0;

    modules_register_filesystem(&filesystem->base);
    filesystem->base.path = "/module/";

}

void nodefs_driver_init(struct nodefs_driver *driver)
{

    memory_clear(driver, sizeof (struct nodefs_driver));

    modules_driver_init(&driver->base, NODEFS_DRIVER_TYPE, "nodefs", 0, 0, 0);
    nodefs_filesystem_init(&driver->filesystem);

    driver->register_node = register_node;
    driver->unregister_node = unregister_node;

}

