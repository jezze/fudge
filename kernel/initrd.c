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

struct tar_header *testFileHeaders[64];
struct vfs_node *testEntries[64];

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

void initrd_parse(unsigned int address, struct vfs_node *initrdNode)
{

    unsigned int inode = 0;

    while (1)
    {

        struct tar_header *header = (struct tar_header *)address;

        if (header->name[0] == '\0' || header->name[0] == ' ')
            break;

        initrdFileHeaders[inode] = header;

        unsigned int size = initrd_get_file_size(header->size);

        struct vfs_node *initrdFileNode = vfs_add_node(header->name + 9, size);
        initrdFileNode->inode = inode;
        initrdFileNode->read = initrd_file_read;
        initrdFileNode->write = initrd_file_write;

        file_write(initrdNode, initrdNode->length, 1, initrdFileNode);

        address += ((size / 512) + 1) * 512;

        if (size % 512)
            address += 512;

        inode++;

    }

}

void initrd_init(unsigned int address)
{

    struct vfs_node *initrdNode = vfs_add_node("initrd", 0);
    initrdNode->walk = initrd_node_walk;
    initrdNode->write = initrd_node_write;

    struct vfs_node *rootNode = call_open("/");
    file_write(rootNode, rootNode->length, 1, initrdNode);

    initrd_parse(address, initrdNode);

}

