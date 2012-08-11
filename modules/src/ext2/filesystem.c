#include <memory.h>
#include <modules.h>
#include <ext2/ext2.h>

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ext2_blockgroup bg;
    struct ext2_node node;
    char mem[1024];
    char *private = mem;
    struct ext2_filesystem *filesystem = (struct ext2_filesystem *)self;
    struct ext2_driver *driver = (struct ext2_driver *)self->driver;

    driver->read_blockgroup(filesystem->device, id, &bg);
    driver->read_node(filesystem->device, id, &bg, &node);
    driver->read_content(filesystem->device, &node, private);

    if ((node.type & 0xF000) == EXT2_NODE_TYPE_DIR)
    {

        char *out = buffer;
        unsigned int length = 0;

        for (;;)
        {

            struct ext2_entry *entry = (struct ext2_entry *)private;

            if (!entry->length)
                return length;

            memory_copy(out + length, private + 8, entry->length);

            if (entry->type == 2)
            {

                memory_copy(out + length + entry->length, "/\n", 2);
                length += entry->length + 2;

            }

            else
            {

                memory_copy(out + length + entry->length, "\n", 1);
                length += entry->length + 1;

            }

            private += entry->size;

        }

        return length;

    }

    if ((node.type & 0xF000) == EXT2_NODE_TYPE_REGULAR)
    {

        memory_copy(buffer, private, node.sizeLow);

        return node.sizeLow;

    }

    return 0;

}

static struct ext2_entry *finddir(struct modules_filesystem *self, unsigned int id, char *name)
{

    struct ext2_blockgroup bg;
    struct ext2_node node;
    char mem[1024];
    char *private = mem;
    struct ext2_filesystem *filesystem = (struct ext2_filesystem *)self;
    struct ext2_driver *driver = (struct ext2_driver *)self->driver;

    driver->read_blockgroup(filesystem->device, id, &bg);
    driver->read_node(filesystem->device, id, &bg, &node);
    driver->read_content(filesystem->device, &node, private);

    if ((node.type & 0xF000) == EXT2_NODE_TYPE_DIR)
    {

        for (;;)
        {

            struct ext2_entry *entry = (struct ext2_entry *)private;

            if (!entry->length)
                return 0;

            if (memory_match(name, private + 8, entry->length))
                return entry;

            private += entry->size;

        }

    }

    return 0;

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    char *temp = buffer;
    struct ext2_entry *entry;
    unsigned int index = 2;

    if (!count)
        return 2;

    while ((entry = finddir(self, index, temp)))
    {

        temp += entry->length + 1;
        index = entry->node;

    }

    return index;

}

void ext2_filesystem_init(struct ext2_filesystem *filesystem, struct ext2_driver *driver, struct mbr_device *device)
{

    memory_clear(filesystem, sizeof (struct ext2_filesystem));

    modules_filesystem_init(&filesystem->base, 0x0001, &driver->base, "hda", 0, 0, read, 0, 0, walk, 0); 

    filesystem->device = device;

}

