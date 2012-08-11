#include <memory.h>
#include <string.h>
#include <modules.h>
#include <nodefs/nodefs.h>

static unsigned int filesystem_read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_node *node;
    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    if (id == 1)
    {

        char *out = buffer;
        unsigned int length = 0;
        unsigned int i;

        for (i = 0; i < filesystem->count; i++)
        {

            unsigned int size = string_length(filesystem->nodes[i]->name);

            memory_copy(out + length, filesystem->nodes[i]->name, size);
            memory_copy(out + length + size, "\n", 1);
            length += size + 1;

        }

        return length;

    }

    node = filesystem->nodes[id - 2];

    return node->read(node, offset, count, buffer);

}

static unsigned int filesystem_write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;
    struct nodefs_node *node = filesystem->nodes[id - 2];

    return node->write(node, offset, count, buffer);

}

static unsigned int filesystem_walk(struct modules_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;
    unsigned int i;

    if (!count)
        return 1;

    for (i = 0; i < filesystem->count; i++)
    {

        if (memory_match(filesystem->nodes[i]->name, buffer, string_length(filesystem->nodes[i]->name) + 1))
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

void nodefs_filesystem_init(struct nodefs_filesystem *filesystem, struct nodefs_driver *driver)
{

    memory_clear(filesystem, sizeof (struct nodefs_filesystem));

    modules_filesystem_init(&filesystem->base, 0x1001, &driver->base, "nodefs", 0, 0, filesystem_read, filesystem_write, 0, filesystem_walk, 0); 
    filesystem->count = 0;

    modules_register_filesystem(&filesystem->base);

}

void nodefs_driver_init(struct nodefs_driver *driver)
{

    memory_clear(driver, sizeof (struct nodefs_driver));

    modules_driver_init(&driver->base, NODEFS_DRIVER_TYPE, "nodefs", 0, 0, 0);
    nodefs_filesystem_init(&driver->filesystem, driver);

    driver->register_node = register_node;
    driver->unregister_node = unregister_node;

}

