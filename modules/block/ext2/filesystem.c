#include <fudge/module.h>
#include <kernel/vfs.h>
#include <base/base.h>
#include <base/block.h>
#include <block/block.h>
#include "ext2.h"

enum ext2_state
{

    EXT2_STATE_CLEAN                    = 1,
    EXT2_STATE_DIRTY                    = 2

};

enum ext2_error
{

    EXT2_ERROR_IGNORE                   = 1,
    EXT2_ERROR_RO                       = 2,
    EXT2_ERROR_PANIC                    = 3

};

enum ext2_osid
{

    EXT2_OSID_LINUX                     = 0,
    EXT2_OSID_HURD                      = 1,
    EXT2_OSID_MASIX                     = 2,
    EXT2_OSID_FREEBSD                   = 3,
    EXT2_OSID_OTHERBSD                  = 4

};

enum ext2_nodetype
{

    EXT2_NODETYPE_FIFO                  = 0x1000,
    EXT2_NODETYPE_CHAR                  = 0x2000,
    EXT2_NODETYPE_DIR                   = 0x4000,
    EXT2_NODETYPE_BLOCK                 = 0x6000,
    EXT2_NODETYPE_REGULAR               = 0x8000,
    EXT2_NODETYPE_SYM                   = 0xA000,
    EXT2_NODETYPE_SOCKET                = 0xC000

};

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ext2_filesystem *filesystem = (struct ext2_filesystem *)self;
    struct ext2_protocol *protocol = (struct ext2_protocol *)filesystem->protocol;
    struct ext2_blockgroup bg;
    struct ext2_node node;
    unsigned char content[1024];
    unsigned char *private = content;

    protocol->read_blockgroup(filesystem->interface, id, &bg);
    protocol->read_node(filesystem->interface, id, &bg, &node);
    protocol->read_content(filesystem->interface, &node, content);

    if ((node.type & 0xF000) == EXT2_NODETYPE_DIR)
    {

        unsigned char *out = buffer;
        unsigned int length = 0;

        for (;;)
        {

            struct ext2_entry *entry = (struct ext2_entry *)private;

            if (!entry->length)
                return length;

            length += memory_read(out + length, 1024, private + 8, entry->length, 0);
            length += (entry->type == 2) ? memory_read(out + length + entry->length, 1024, "/\n", 2, 0) : memory_read(out + length + entry->length, 1024, "\n", 1, 0);
            private += entry->size;

        }

        return length;

    }

    if ((node.type & 0xF000) == EXT2_NODETYPE_REGULAR)
        return memory_read(buffer, count, content, node.sizeLow, offset);

    return 0;

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static struct ext2_entry *finddir(struct vfs_backend *backend, unsigned int id, const char *name)
{

/*
    struct ext2_filesystem *filesystem = (struct ext2_filesystem *)self;
    struct ext2_protocol *protocol = (struct ext2_protocol *)filesystem->protocol;
    struct ext2_blockgroup bg;
    struct ext2_node node;
    unsigned char content[1024];
    unsigned char *private = content;

    protocol->read_blockgroup(filesystem->interface, id, &bg);
    protocol->read_node(filesystem->interface, id, &bg, &node);
    protocol->read_content(filesystem->interface, &node, private);

    if ((node.type & 0xF000) == EXT2_NODETYPE_DIR)
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
*/
    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    struct ext2_entry *entry;

    if (!count)
        return id;

    while ((entry = finddir(backend, id, path)))
    {

        path += entry->length + 1;
        id = entry->node;

    }

    return id;

}

void ext2_init_filesystem(struct ext2_filesystem *filesystem, struct ext2_protocol *protocol, struct base_block *interface)
{

    vfs_init_protocol(&filesystem->base, 2, 0, open, close, read, write, 0, walk, 0);

    filesystem->protocol = protocol;
    filesystem->interface = interface;

}

