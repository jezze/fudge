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
struct vfs_node *initrdTestEntries[64];

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

static struct vfs_node *initrd_node_walk(struct vfs_node *node, unsigned int index)
{

    return (index < node->length) ? initrdEntries[index] : 0;

}

static unsigned int initrd_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    initrdEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

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

static void initrd_create_nodes(struct vfs_node *rootNode, unsigned int numEntries)
{

    unsigned int i;

    for (i = 0; i < numEntries; i++)
    {

        struct tar_header *header = initrdFileHeaders[i];

        unsigned int size = initrd_get_file_size(header->size);

        struct vfs_node *initrdFileNode = vfs_add_node(header->name + 9, size);
        initrdFileNode->inode = i;
        initrdFileNode->read = initrd_file_read;
        initrdFileNode->write = initrd_file_write;

        file_write(rootNode, rootNode->length, 1, initrdFileNode);

    }

}

static struct vfs_node *initrd_node_walk2(struct vfs_node *node, unsigned int index)
{

    return (index < node->length) ? initrdTestEntries[node->inode + index + 1] : 0;

}

static unsigned int initrd_node_write2(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    initrdTestEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

static unsigned int initrd_create_nodes2(struct vfs_node *rootNode, unsigned int i, unsigned int numEntries)
{

    for (; i < numEntries; i++)
    {

        struct tar_header *header = initrdFileHeaders[i];

        unsigned int size = initrd_get_file_size(header->size);

        unsigned int skip = (size) ? 0 : 1;
        unsigned int start = string_index_reversed(header->name, '/', skip) + 1;

        struct vfs_node *initrdFileNode = vfs_add_node(header->name + start, size);
        initrdFileNode->inode = i;
        string_replace(initrdFileNode->name, '/', '\0');

        file_write(rootNode, rootNode->length, 1, initrdFileNode);

        if (size)
        {

            initrdFileNode->read = initrd_file_read;
            initrdFileNode->write = initrd_file_write;

        }

        else
        {

            initrdFileNode->walk = initrd_node_walk2;
            initrdFileNode->write = initrd_node_write2;

            i = initrd_create_nodes2(initrdFileNode, i + 1, numEntries);

        }

    }

    return i;

}

void initrd_init(unsigned int address)
{

    unsigned int numEntries = initrd_parse(address);

    struct vfs_node *initrdNode = vfs_add_node("initrd", 0);
    initrdNode->walk = initrd_node_walk;
    initrdNode->write = initrd_node_write;

    struct vfs_node *rootNode = call_open("/");
    file_write(rootNode, rootNode->length, 1, initrdNode);

    initrd_create_nodes(initrdNode, numEntries);
    initrd_create_nodes2(rootNode, 0, numEntries);

}

