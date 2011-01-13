#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/initrd.h>
#include <kernel/vfs.h>

struct vfs_node *initrdEntries[32];

struct initrd_header *initrdHeader;
struct initrd_file_header *initrdFileHeaders;

static unsigned int initrd_file_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct initrd_file_header header = initrdFileHeaders[node->inode];

    if (offset > header.length)
        return 0;

    if (offset + count > header.length)
        count = header.length - offset;

    memory_copy(buffer, (unsigned char *)(header.offset + offset), count);

    return count;

}

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

static struct vfs_node *initrd_node_walk(struct vfs_node *node, unsigned int index)
{

    if (index < node->length)
        return initrdEntries[index];
    else
        return 0;

}

static unsigned int initrd_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    initrdEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

void initrd_init(struct initrd_header *header)
{

    struct vfs_node *initrdNode = vfs_add_node("initrd", 0);
    initrdNode->walk = initrd_node_walk;
    initrdNode->write = initrd_node_write;

    initrdFileHeaders = (struct initrd_file_header *)((unsigned int)header + sizeof (struct initrd_header));

    unsigned int i;

    for (i = 0; i < header->nfiles; i++)
    {

        initrdFileHeaders[i].offset += (unsigned int)header;

        struct vfs_node *initrdFileNode = vfs_add_node(initrdFileHeaders[i].name, initrdFileHeaders[i].length);
        initrdFileNode->inode = i;
        initrdFileNode->read = initrd_file_read;
        initrdFileNode->write = initrd_file_write;

        file_write(initrdNode, initrdNode->length, 1, initrdFileNode);

    }

    struct vfs_node *rootNode = call_open("/");
    file_write(rootNode, rootNode->length, 1, initrdNode);

}

