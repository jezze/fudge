#include <fudge/module.h>
#include <base/base.h>
#include <base/block.h>
#include <block/block.h>
#include "ext2.h"

#define STATECLEAN                      1
#define STATEDIRTY                      2
#define ERRORIGNORE                     1
#define ERRORRO                         2
#define ERRORPANIC                      3
#define OSIDLINUX                       0
#define OSIDHURD                        1
#define OSIDMASIX                       2
#define OSIDFREEBSD                     3
#define OSIDOTHERBSD                    4
#define NODETYPEFIFO                    0x1000
#define NODETYPECHAR                    0x2000
#define NODETYPEDIR                     0x4000
#define NODETYPEBLOCK                   0x6000
#define NODETYPEREGULAR                 0x8000
#define NODETYPESYM                     0xA000
#define NODETYPESOCKET                  0xC000

static unsigned int open(struct service_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int close(struct service_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int read(struct service_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
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

    if ((node.type & 0xF000) == NODETYPEDIR)
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

    if ((node.type & 0xF000) == NODETYPEREGULAR)
        return memory_read(buffer, count, content, node.sizeLow, offset);

    return 0;

}

static unsigned int write(struct service_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static struct ext2_entry *finddir(struct service_backend *backend, unsigned int id, char *name)
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

    if ((node.type & 0xF000) == NODETYPEDIR)
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

static unsigned int walk(struct service_backend *backend, unsigned int id, unsigned int count, char *path)
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

    service_init_protocol(&filesystem->base, 2, 0, open, close, read, write, 0, walk, 0);

    filesystem->protocol = protocol;
    filesystem->interface = interface;

}

