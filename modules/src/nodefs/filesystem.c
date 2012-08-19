#include <memory.h>
#include <string.h>
#include <modules.h>
#include <nodefs/nodefs.h>

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_node *node;
    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;

    if (id == 1)
    {

        char *out = buffer;
        unsigned int length = 0;
        unsigned int i;

        for (i = 0; i < filesystem->driver->nodesCount; i++)
        {

            unsigned int size = string_length(filesystem->driver->nodes[i]->name);

            memory_copy(out + length, filesystem->driver->nodes[i]->name, size);
            memory_copy(out + length + size, "\n", 1);
            length += size + 1;

        }

        return length;

    }

    node = filesystem->driver->nodes[id - 2];

    return node->read(node, offset, count, buffer);

}

static unsigned int write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;
    struct nodefs_node *node = filesystem->driver->nodes[id - 2];

    return node->write(node, offset, count, buffer);

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    struct nodefs_filesystem *filesystem = (struct nodefs_filesystem *)self;
    unsigned int i;

    if (!count)
        return id;

    for (i = 0; i < filesystem->driver->nodesCount; i++)
    {

        if (memory_match(filesystem->driver->nodes[i]->name, path, string_length(filesystem->driver->nodes[i]->name) + 1))
            return i + 2;

    }

    return 0;

}

void nodefs_filesystem_init(struct nodefs_filesystem *filesystem, struct nodefs_driver *driver)
{

    memory_clear(filesystem, sizeof (struct nodefs_filesystem));

    modules_filesystem_init(&filesystem->base, 0x1001, 1, "nodefs", 0, 0, read, write, walk, 0);

    filesystem->driver = driver;

}

