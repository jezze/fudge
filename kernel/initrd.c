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
/*
static unsigned int initrd_file_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct initrd_file_header header = initrdFileHeaders[node->inode];

    if (offset > header.length)
        return 0;

    if (offset + count > header.length)
        count = header.length - offset;

    memory_copy((unsigned char *)(header.offset + offset), buffer, count);

    return count;

}
*/
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

/*
void initrd_init(unsigned int address)
{

    struct vfs_node *initrdNode = vfs_add_node("initrd", 0);
    initrdNode->walk = initrd_node_walk;
    initrdNode->write = initrd_node_write;

    initrdHeader = (struct initrd_header*)address;
    initrdFileHeaders = (struct initrd_file_header *)(address + sizeof (struct initrd_header));

    unsigned int i;

    for (i = 0; i < initrdHeader->nfiles; i++)
    {

        initrdFileHeaders[i].offset += address;

        struct vfs_node *initrdFileNode = vfs_add_node(initrdFileHeaders[i].name, initrdFileHeaders[i].length);
        initrdFileNode->inode = i;
        initrdFileNode->read = initrd_file_read;
        initrdFileNode->write = initrd_file_write;

        file_write(initrdNode, initrdNode->length, 1, initrdFileNode);

    }

    struct vfs_node *rootNode = call_open("/");
    file_write(rootNode, rootNode->length, 1, initrdNode);

}
*/

void initrd_init(unsigned int address)
{

    struct vfs_node *initrdNode = vfs_add_node("initrd", 0);
    initrdNode->walk = initrd_node_walk;
    initrdNode->write = initrd_node_write;

    unsigned int offset = 0;
    unsigned int inode = 0;

    while (1)
    {

        struct tar_header *header = (struct tar_header *)(address + offset);

        if (header->name[0] == '\0' || header->name[0] == ' ')
            break;

        initrdFileHeaders[inode] = header;

        unsigned int j;
        unsigned int count = 1;
        unsigned int size = 0;

        for (j = 11; j > 0; j--, count *= 8)
            size += ((header->size[j - 1] - '0') * count);

        struct vfs_node *initrdFileNode = vfs_add_node(header->name + 9, size);
        initrdFileNode->inode = inode;
        initrdFileNode->read = initrd_file_read;
//        initrdFileNode->write = initrd_file_write;

        file_write(initrdNode, initrdNode->length, 1, initrdFileNode);

        offset += ((size / 512) + 1) * 512;

        if (size % 512)
            offset += 512;

        inode++;

    }

    struct vfs_node *rootNode = call_open("/");
    file_write(rootNode, rootNode->length, 1, initrdNode);

}
