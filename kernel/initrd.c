#include <lib/cbuffer.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/initrd.h>

struct initrd_header *initrdHeader;
struct initrd_file_header *initrdFileHeaders;

struct vfs_node root;
struct vfs_node rootNodes[32];
unsigned int rootNodesCount;

struct vfs_node devNodes[32];
unsigned int devNodesCount;

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

    if (index < rootNodesCount)
        return &rootNodes[index];
    else
        return 0;

}

static struct vfs_node *dev_walk(struct vfs_node *node, unsigned int index)
{

    if (index < devNodesCount)
        return &devNodes[index];
    else
        return 0;

}

struct vfs_node *initrd_init(unsigned int location)
{

    // ROOT

    string_copy(root.name, "root");
    root.inode = 0;
    root.length = 0;
    root.open = 0;
    root.close = 0;
    root.read = 0;
    root.write = 0;
    root.walk = initrd_walk;

    rootNodesCount = 0;

    string_copy(rootNodes[rootNodesCount].name, ".");
    rootNodes[rootNodesCount].inode = 0;
    rootNodes[rootNodesCount].length = 0;
    rootNodes[rootNodesCount].open = 0;
    rootNodes[rootNodesCount].close = 0;
    rootNodes[rootNodesCount].read = 0;
    rootNodes[rootNodesCount].write = 0;
    rootNodes[rootNodesCount].walk = initrd_walk;
    rootNodesCount++;

    string_copy(rootNodes[rootNodesCount].name, "..");
    rootNodes[rootNodesCount].inode = 0;
    rootNodes[rootNodesCount].length = 0;
    rootNodes[rootNodesCount].open = 0;
    rootNodes[rootNodesCount].close = 0;
    rootNodes[rootNodesCount].read = 0;
    rootNodes[rootNodesCount].write = 0;
    rootNodes[rootNodesCount].walk = initrd_walk;
    rootNodesCount++;

    string_copy(rootNodes[rootNodesCount].name, "dev");
    rootNodes[rootNodesCount].inode = 0;
    rootNodes[rootNodesCount].length = 0;
    rootNodes[rootNodesCount].open = 0;
    rootNodes[rootNodesCount].close = 0;
    rootNodes[rootNodesCount].read = 0;
    rootNodes[rootNodesCount].write = 0;
    rootNodes[rootNodesCount].walk = dev_walk;
    rootNodesCount++;

    arch_set_stdin(&rootNodes[rootNodesCount]);
    rootNodesCount++;

    arch_set_stdout(&rootNodes[rootNodesCount]);
    rootNodesCount++;

    // INITRD

    initrdHeader = (struct initrd_header *)location;
    initrdFileHeaders = (struct initrd_file_header *)(location + sizeof (struct initrd_header));

    unsigned int i;

    for (i = 0; i < initrdHeader->nfiles; i++)
    {

        initrdFileHeaders[i].offset += location;

        string_copy(rootNodes[rootNodesCount].name, initrdFileHeaders[i].name);
        rootNodes[rootNodesCount].inode = i;
        rootNodes[rootNodesCount].length = initrdFileHeaders[i].length;
        rootNodes[rootNodesCount].open = 0;
        rootNodes[rootNodesCount].close = 0;
        rootNodes[rootNodesCount].read = initrd_read;
        rootNodes[rootNodesCount].write = 0;
        rootNodes[rootNodesCount].walk = 0;

        rootNodesCount++;

    }

    // DEV

    devNodesCount = 0;

    string_copy(devNodes[devNodesCount].name, ".");
    devNodes[devNodesCount].inode = 0;
    devNodes[devNodesCount].length = 0;
    devNodes[devNodesCount].open = 0;
    devNodes[devNodesCount].close = 0;
    devNodes[devNodesCount].read = 0;
    devNodes[devNodesCount].write = 0;
    devNodes[devNodesCount].walk = dev_walk;
    devNodesCount++;

    string_copy(devNodes[devNodesCount].name, "..");
    devNodes[devNodesCount].inode = 0;
    devNodes[devNodesCount].length = 0;
    devNodes[devNodesCount].open = 0;
    devNodes[devNodesCount].close = 0;
    devNodes[devNodesCount].read = 0;
    devNodes[devNodesCount].write = 0;
    devNodes[devNodesCount].walk = initrd_walk;
    devNodesCount++;

    string_copy(devNodes[devNodesCount].name, "file1");
    devNodes[devNodesCount].inode = 0;
    devNodes[devNodesCount].length = 0;
    devNodes[devNodesCount].open = 0;
    devNodes[devNodesCount].close = 0;
    devNodes[devNodesCount].read = 0;
    devNodes[devNodesCount].write = 0;
    devNodes[devNodesCount].walk = 0;
    devNodesCount++;

    string_copy(devNodes[devNodesCount].name, "file2");
    devNodes[devNodesCount].inode = 0;
    devNodes[devNodesCount].length = 0;
    devNodes[devNodesCount].open = 0;
    devNodes[devNodesCount].close = 0;
    devNodes[devNodesCount].read = 0;
    devNodes[devNodesCount].write = 0;
    devNodes[devNodesCount].walk = 0;
    devNodesCount++;

    return &root;

}

