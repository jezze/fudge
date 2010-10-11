#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/initrd.h>

struct vfs_node initrdNode;
struct vfs_node *initrdEntries[32];

struct initrd_header *initrdHeader;
struct initrd_file_header *initrdFileHeaders;

struct vfs_node initrdFiles[32];
unsigned int initrdFilesCount;

static unsigned int initrd_file_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct initrd_file_header header = initrdFileHeaders[node->inode];

    if (offset > header.length)
        return 0;

    if (offset + count > header.length)
        count = header.length - offset;

    memory_copy(buffer, (unsigned char*)(header.offset + offset), count);

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

void initrd_init(unsigned int location)
{

    memory_set(&initrdNode, 0, sizeof (struct vfs_node));
    string_copy(initrdNode.name, "initrd");
    initrdNode.walk = initrd_node_walk;
    initrdNode.write = initrd_node_write;

    initrdFilesCount = 0;

    initrdHeader = (struct initrd_header *)location;
    initrdFileHeaders = (struct initrd_file_header *)(location + sizeof (struct initrd_header));

    unsigned int i;

    for (i = 0; i < initrdHeader->nfiles; i++)
    {

        initrdFileHeaders[i].offset += location;

        memory_set(&initrdFiles[initrdFilesCount], 0, sizeof (struct vfs_node));
        string_copy(initrdFiles[initrdFilesCount].name, initrdFileHeaders[i].name);
        initrdFiles[initrdFilesCount].inode = i;
        initrdFiles[initrdFilesCount].length = initrdFileHeaders[i].length;
        initrdFiles[initrdFilesCount].read = initrd_file_read;

        vfs_write(&initrdNode, initrdNode.length, 1, &initrdFiles[initrdFilesCount]);

        initrdFilesCount++;

    }

    struct vfs_node *root = call_open(".");
    vfs_write(root, root->length, 1, &initrdNode);

}

