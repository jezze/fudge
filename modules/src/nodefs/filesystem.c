#include <memory.h>
#include <string.h>
#include <modules.h>
#include <nodefs/nodefs.h>

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_node *node;
    struct nodefs_driver *driver = (struct nodefs_driver *)self->driver;

    if (id == 1)
    {

        char *out = buffer;
        unsigned int length = 0;
        unsigned int i;

        for (i = 0; i < driver->nodesCount; i++)
        {

            unsigned int size = string_length(driver->nodes[i]->name);

            memory_copy(out + length, driver->nodes[i]->name, size);
            memory_copy(out + length + size, "\n", 1);
            length += size + 1;

        }

        return length;

    }

    node = driver->nodes[id - 2];

    return node->read(node, offset, count, buffer);

}

static unsigned int write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_driver *driver = (struct nodefs_driver *)self->driver;
    struct nodefs_node *node = driver->nodes[id - 2];

    return node->write(node, offset, count, buffer);

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    struct nodefs_driver *driver = (struct nodefs_driver *)self->driver;
    unsigned int i;

    if (!count)
        return 1;

    for (i = 0; i < driver->nodesCount; i++)
    {

        if (memory_match(driver->nodes[i]->name, buffer, string_length(driver->nodes[i]->name) + 1))
            return i + 2;

    }

    return 0;

}

void nodefs_filesystem_init(struct modules_filesystem *filesystem, struct modules_driver *driver)
{

    memory_clear(filesystem, sizeof (struct modules_filesystem));

    modules_filesystem_init(filesystem, 0x1001, driver, "nodefs", 0, 0, read, write, 0, walk, 0); 

}

