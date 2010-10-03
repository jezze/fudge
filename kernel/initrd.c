#include <lib/call.h>
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

    if (index < node->length)
        return initrdEntries[index];
    else
        return 0;

}

static unsigned int initrd_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    initrdEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

void initrd_init(unsigned int location)
{

    memory_set(&initrd, 0, sizeof (struct vfs_node));
    string_copy(initrd.name, "initrd");
    initrd.write = initrd_write;
    initrd.walk = initrd_walk;

    initrdNodesCount = 0;

    initrdHeader = (struct initrd_header *)location;
    initrdFileHeaders = (struct initrd_file_header *)(location + sizeof (struct initrd_header));

    unsigned int i;

    for (i = 0; i < initrdHeader->nfiles; i++)
    {

        initrdFileHeaders[i].offset += location;

        memory_set(&initrdNodes[initrdNodesCount], 0, sizeof (struct vfs_node));
        string_copy(initrdNodes[initrdNodesCount].name, initrdFileHeaders[i].name);
        initrdNodes[initrdNodesCount].inode = i;
        initrdNodes[initrdNodesCount].length = initrdFileHeaders[i].length;
        initrdNodes[initrdNodesCount].read = initrd_read;

        vfs_write(&initrd, initrd.length, 1, &initrdNodes[initrdNodesCount]);

        initrdNodesCount++;

    }

    struct vfs_node *root = call_vfs_find(".");
    vfs_write(root, root->length, 1, &initrd);

}

