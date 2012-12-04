#include <memory.h>
#include <vfs.h>
#include <block/block.h>
#include <ext2/ext2.h>

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ext2_blockgroup bg;
    struct ext2_node node;
    char content[1024];
    char *private = content;
    struct ext2_filesystem *filesystem = (struct ext2_filesystem *)self;
    struct ext2_protocol *protocol = (struct ext2_protocol *)filesystem->protocol;

    protocol->read_blockgroup(filesystem->interface, id, &bg);
    protocol->read_node(filesystem->interface, id, &bg, &node);
    protocol->read_content(filesystem->interface, &node, content);

    if ((node.type & 0xF000) == EXT2_NODE_TYPE_DIR)
    {

        char *out = buffer;
        unsigned int length = 0;

        for (;;)
        {

            struct ext2_entry *entry = (struct ext2_entry *)private;

            if (!entry->length)
                return length;

            length += memory_read(out + length, 1024, private + 8, entry->length, 0);

            if (entry->type == 2)
                length += memory_read(out + length + entry->length, 1024, "/\n", 2, 0);
            else
                length += memory_read(out + length + entry->length, 1024, "\n", 1, 0);

            private += entry->size;

        }

        return length;

    }

    if ((node.type & 0xF000) == EXT2_NODE_TYPE_REGULAR)
        return memory_read(buffer, count, content, node.sizeLow, offset);

    return 0;

}

static struct ext2_entry *finddir(struct vfs_interface *self, unsigned int id, const char *name)
{

    struct ext2_blockgroup bg;
    struct ext2_node node;
    char content[1024];
    char *private = content;
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

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    struct ext2_entry *entry;

    if (!count)
        return id;

    while ((entry = finddir(self, id, path)))
    {

        path += entry->length + 1;
        id = entry->node;

    }

    return id;

}

void ext2_init_filesystem(struct ext2_filesystem *filesystem, struct ext2_protocol *protocol, struct block_interface *interface)
{

    memory_clear(filesystem, sizeof (struct ext2_filesystem));

    vfs_init_interface(&filesystem->base, 2, "hda", 0, 0, read, 0, walk, 0);

    filesystem->protocol = protocol;
    filesystem->interface = interface;

}

