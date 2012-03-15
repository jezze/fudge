#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver *driver;
static struct vfs_filesystem filesystem;

static unsigned int read(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ext2_blockgroup bg;
    struct ext2_node node;
    char mem[1024];
    void *private = mem;

    driver->read_blockgroup(driver, id, &bg);
    driver->read_node(driver, id, &bg, &node);
    driver->read_content(driver, &node, private);

    if ((node.type & 0xF000) == EXT2_NODE_TYPE_DIR)
    {

        unsigned int c = 0;

        for (;;)
        {

            struct ext2_entry *entry = private;

            if (!entry->length)
                return c;

            memory_copy(buffer + c, private + 8, entry->length);
            memory_copy(buffer + c + entry->length, "\n", 1);
            c += entry->length + 1;

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

    struct ext2_blockgroup bg;
    struct ext2_node node;
    char mem[1024];
    void *private = mem;

    driver->read_blockgroup(driver, id, &bg);
    driver->read_node(driver, id, &bg, &node);
    driver->read_content(driver, &node, private);

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

void ext2_filesystem_init(struct modules_module *module)
{

    driver = (struct ext2_driver *)module;

    vfs_filesystem_init(&filesystem, 0, 0, read, 0, find, 0); 
    vfs_mount(&filesystem, "/hda/");

}

