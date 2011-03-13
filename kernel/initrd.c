#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <lib/vfs.h>
#include <kernel/initrd.h>
#include <kernel/vfs.h>

struct tar_header *initrdFileHeaders[64];
struct vfs_node *initrdEntries[64];

static unsigned int initrd_file_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct tar_header *header = initrdFileHeaders[node->inode];

    if (offset > node->length)
        return 0;

    if (offset + count > node->length)
        count = node->length - offset;

    unsigned int address = (unsigned int)header + 512;

    memory_copy(buffer, (unsigned char *)(address + offset), count);

    return count;

}

static unsigned int initrd_file_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct tar_header *header = initrdFileHeaders[node->inode];

    if (offset > node->length)
        return 0;

    if (offset + count > node->length)
        count = node->length - offset;

    unsigned int address = (unsigned int)header + 512;

    memory_copy((unsigned char *)(address + offset), buffer, count);

    return count;

}

static unsigned int initrd_get_file_size(const char *in)
{

    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

}

static unsigned int initrd_parse(unsigned int address)
{

    unsigned int i;

    for (i = 0; ; i++)
    {

        struct tar_header *header = (struct tar_header *)address;

        if (header->name[0] == '\0' || header->name[0] == ' ')
            break;

        unsigned int size = initrd_get_file_size(header->size);

        initrdFileHeaders[i] = header;

        address += ((size / 512) + 1) * 512;

        if (size % 512)
            address += 512;

    }

    return i;

}

static struct vfs_node *initrd_node_walk(struct vfs_node *node, unsigned int index)
{

    return (index < node->length) ? initrdEntries[node->inode + index + 1] : 0;

}

static unsigned int initrd_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    node->length++;

    return count;

}

static void initrd_create_nodes(unsigned int numEntries)
{

    unsigned int i;

    for (i = 0; i < numEntries; i++)
    {

        struct tar_header *header = initrdFileHeaders[i];
        unsigned int size = initrd_get_file_size(header->size);
        unsigned int start = string_index_reversed(header->name, '/', (size) ? 0 : 1) + 1;

        struct vfs_node *initrdFileNode = vfs_add_node(header->name + start, size);
        initrdFileNode->inode = i;

        initrdEntries[i] = initrdFileNode;

        if (size)
        {

            initrdFileNode->read = initrd_file_read;
            initrdFileNode->write = initrd_file_write;

        }

        else
        {

            string_replace(initrdFileNode->name, '/', '\0');
            initrdFileNode->walk = initrd_node_walk;
            initrdFileNode->write = initrd_node_write;

        }

        char baseName[256];
        memory_copy(baseName, header->name + 4, start);
        baseName[start - 4] = '\0';

        struct vfs_node *rootNode = call_open(baseName);
        file_write(rootNode, rootNode->length, 1, initrdFileNode);

    }

}

void initrd_init(unsigned int address)
{

    unsigned int numEntries = initrd_parse(address + 512);

    initrd_create_nodes(numEntries);

}

