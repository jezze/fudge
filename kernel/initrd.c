#include <lib/call.h>
#include <lib/cbuffer.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/initrd.h>

struct vfs_node initrd;
struct vfs_node *initrdEntries[32];

struct initrd_header *initrdHeader;
struct initrd_file_header *initrdFileHeaders;

struct vfs_node initrdNodes[32];
unsigned int initrdNodesCount;

static unsigned int initrd_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct initrd_file_header header = initrdFileHeaders[node->inode];

    if (offset > header.length)
        return 0;

    if (offset + count > header.length)
        count = header.length - offset;

    memory_copy(buffer, (unsigned char*)(header.offset + offset), count);

    return count;

}

static struct vfs_node *initrd_walk(struct vfs_node *node, unsigned int index)
{

    if (index < initrd.length)
        return initrdEntries[index];
    else
        return 0;

}

static unsigned int initrd_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    initrdEntries[initrd.length] = (struct vfs_node *)buffer;
    initrd.length++;

    return count;

}

void initrd_init(unsigned int location)
{

    string_copy(initrd.name, "initrd");
    initrd.inode = 0;
    initrd.length = 0;
    initrd.open = 0;
    initrd.close = 0;
    initrd.read = 0;
    initrd.write = initrd_write;
    initrd.walk = initrd_walk;

    initrdNodesCount = 0;

    initrdHeader = (struct initrd_header *)location;
    initrdFileHeaders = (struct initrd_file_header *)(location + sizeof (struct initrd_header));

    unsigned int i;

    for (i = 0; i < initrdHeader->nfiles; i++)
    {

        initrdFileHeaders[i].offset += location;

        string_copy(initrdNodes[initrdNodesCount].name, initrdFileHeaders[i].name);
        initrdNodes[initrdNodesCount].inode = i;
        initrdNodes[initrdNodesCount].length = initrdFileHeaders[i].length;
        initrdNodes[initrdNodesCount].open = 0;
        initrdNodes[initrdNodesCount].close = 0;
        initrdNodes[initrdNodesCount].read = initrd_read;
        initrdNodes[initrdNodesCount].write = 0;
        initrdNodes[initrdNodesCount].walk = 0;

        vfs_write(&initrd, 0, 1, &initrdNodes[initrdNodesCount]);

        initrdNodesCount++;

    }

    struct vfs_node *root = call_vfs_find(".");
    vfs_write(root, 0, 1, &initrd);

}

