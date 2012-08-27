#include <memory.h>
#include <vfs.h>
#include <block/block.h>
#include <ext2/ext2.h>

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ext2_blockgroup bg;
    struct ext2_node node;
    char mem[1024];
    char *private = mem;
    struct ext2_filesystem *filesystem = (struct ext2_filesystem *)self;
    struct ext2_protocol *protocol = (struct ext2_protocol *)filesystem->protocol;

    protocol->read_blockgroup(filesystem->interface, id, &bg);
    protocol->read_node(filesystem->interface, id, &bg, &node);
    protocol->read_content(filesystem->interface, &node, private);

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

static struct ext2_entry *finddir(struct vfs_interface *self, unsigned int id, char *name)
{

    struct ext2_blockgroup bg;
    struct ext2_node node;
    char mem[1024];
    char *private = mem;
    struct ext2_filesystem *filesystem = (struct ext2_filesystem *)self;
    struct ext2_protocol *protocol = (struct ext2_protocol *)filesystem->protocol;

    protocol->read_blockgroup(filesystem->interface, id, &bg);
    protocol->read_node(filesystem->interface, id, &bg, &node);
    protocol->read_content(filesystem->interface, &node, private);

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

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, char *path)
{

    char *temp = path;
    struct ext2_entry *entry;

    if (!count)
        return id;

    while ((entry = finddir(self, id, temp)))
    {

        temp += entry->length + 1;
        id = entry->node;

    }

    return id;

}

void ext2_filesystem_init(struct ext2_filesystem *filesystem, struct ext2_protocol *protocol, struct block_interface *interface)
{

    memory_clear(filesystem, sizeof (struct ext2_filesystem));

    vfs_interface_init(&filesystem->base, 2, "hda", 0, 0, read, 0, walk, 0);

    filesystem->protocol = protocol;
    filesystem->interface = interface;

}

