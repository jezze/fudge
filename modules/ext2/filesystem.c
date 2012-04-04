#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/ext2/ext2.h>

static unsigned int read(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ext2_filesystem *filesystem = (struct ext2_filesystem *)self;

    struct ext2_blockgroup bg;
    struct ext2_node node;
    char mem[1024];
    void *private = mem;

    filesystem->driver->read_blockgroup(filesystem->driver, id, &bg);
    filesystem->driver->read_node(filesystem->driver, id, &bg, &node);
    filesystem->driver->read_content(filesystem->driver, &node, private);

    if ((node.type & 0xF000) == EXT2_NODE_TYPE_DIR)
    {

        unsigned int c = 0;

        for (;;)
        {

            struct ext2_entry *entry = private;

            if (!entry->length)
                return c;

            memory_copy(buffer + c, private + 8, entry->length);

            if (entry->type == 2)
            {

                memory_copy(buffer + c + entry->length, "/\n", 2);
                c += entry->length + 2;

            }

            else
            {

                memory_copy(buffer + c + entry->length, "\n", 1);
                c += entry->length + 1;

            }

            private += entry->size;

        }

        return c;

    }

    if ((node.type & 0xF000) == EXT2_NODE_TYPE_REGULAR)
    {

        memory_copy(buffer, private, node.sizeLow);

        return node.sizeLow;

    }

    return 0;

}

static struct ext2_entry *finddir(struct vfs_filesystem *self, unsigned int id, char *name)
{

    struct ext2_filesystem *filesystem = (struct ext2_filesystem *)self;

    struct ext2_blockgroup bg;
    struct ext2_node node;
    char mem[1024];
    void *private = mem;

    filesystem->driver->read_blockgroup(filesystem->driver, id, &bg);
    filesystem->driver->read_node(filesystem->driver, id, &bg, &node);
    filesystem->driver->read_content(filesystem->driver, &node, private);

    if ((node.type & 0xF000) == EXT2_NODE_TYPE_DIR)
    {

        for (;;)
        {

            struct ext2_entry *entry = private;

            if (!entry->length)
                return 0;

            if (!memory_compare(name, private + 8, entry->length))
                return entry;

            private += entry->size;

        }

    }

    return 0;

}

static unsigned int find(struct vfs_filesystem *self, char *name)
{

    unsigned int length = string_length(name);

    if (!length)
        return 2;

    char *temp = name;
    struct ext2_entry *entry;
    unsigned int id = 2;

    while ((entry = finddir(self, id, temp)))
    {

        temp += entry->length + 1;
        id = entry->node;

    }

    return id;

}

void ext2_filesystem_init(struct ext2_filesystem *filesystem, struct ext2_driver *driver)
{

    memory_clear(filesystem, sizeof (struct ext2_filesystem));

    vfs_filesystem_init(&filesystem->base, 0, 0, read, 0, find, 0); 
    vfs_mount(&filesystem->base, "/hda/");

    filesystem->driver = driver;

}

