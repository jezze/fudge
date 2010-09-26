#include <lib/cbuffer.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/vfs.h>

struct vfs_node root;
struct vfs_node rootNodes[32];
unsigned int rootNodesCount;

struct vfs_node devNodes[32];
unsigned int devNodesCount;

struct initrd_header *initrdHeader;
struct initrd_file_header *initrdFileHeaders;

struct vfs_node initrdNodes[32];
unsigned int initrdNodesCount;

static struct vfs_node *vfs_root_walk(struct vfs_node *node, unsigned int index)
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

    if (index < initrdNodesCount)
        return &initrdNodes[index];
    else
        return 0;

}

void dev_init()
{

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
    devNodes[devNodesCount].walk = vfs_root_walk;
    devNodesCount++;

    string_copy(devNodes[devNodesCount].name, "stdin");
    devNodes[devNodesCount].inode = 0;
    devNodes[devNodesCount].length = 0;
    devNodes[devNodesCount].open = 0;
    devNodes[devNodesCount].close = 0;
    devNodes[devNodesCount].read = 0;
    devNodes[devNodesCount].write = 0;
    devNodes[devNodesCount].walk = 0;
    devNodesCount++;

    string_copy(devNodes[devNodesCount].name, "stdout");
    devNodes[devNodesCount].inode = 0;
    devNodes[devNodesCount].length = 0;
    devNodes[devNodesCount].open = 0;
    devNodes[devNodesCount].close = 0;
    devNodes[devNodesCount].read = 0;
    devNodes[devNodesCount].write = 0;
    devNodes[devNodesCount].walk = 0;
    devNodesCount++;

    string_copy(devNodes[devNodesCount].name, "rtc");
    devNodes[devNodesCount].inode = 0;
    devNodes[devNodesCount].length = 0;
    devNodes[devNodesCount].open = 0;
    devNodes[devNodesCount].close = 0;
    devNodes[devNodesCount].read = 0;
    devNodes[devNodesCount].write = 0;
    devNodes[devNodesCount].walk = 0;
    devNodesCount++;

}

void initrd_init(unsigned int location)
{

    initrdNodesCount = 0;

    string_copy(initrdNodes[initrdNodesCount].name, ".");
    initrdNodes[initrdNodesCount].inode = 0;
    initrdNodes[initrdNodesCount].length = 0;
    initrdNodes[initrdNodesCount].open = 0;
    initrdNodes[initrdNodesCount].close = 0;
    initrdNodes[initrdNodesCount].read = 0;
    initrdNodes[initrdNodesCount].write = 0;
    initrdNodes[initrdNodesCount].walk = initrd_walk;
    initrdNodesCount++;

    string_copy(initrdNodes[initrdNodesCount].name, "..");
    initrdNodes[initrdNodesCount].inode = 0;
    initrdNodes[initrdNodesCount].length = 0;
    initrdNodes[initrdNodesCount].open = 0;
    initrdNodes[initrdNodesCount].close = 0;
    initrdNodes[initrdNodesCount].read = 0;
    initrdNodes[initrdNodesCount].write = 0;
    initrdNodes[initrdNodesCount].walk = vfs_root_walk;
    initrdNodesCount++;

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

        initrdNodesCount++;

    }

}

struct vfs_node *vfs_init(unsigned int location)
{

    // ROOT

    string_copy(root.name, "root");
    root.inode = 0;
    root.length = 0;
    root.open = 0;
    root.close = 0;
    root.read = 0;
    root.write = 0;
    root.walk = vfs_root_walk;

    rootNodesCount = 0;

    string_copy(rootNodes[rootNodesCount].name, ".");
    rootNodes[rootNodesCount].inode = 0;
    rootNodes[rootNodesCount].length = 0;
    rootNodes[rootNodesCount].open = 0;
    rootNodes[rootNodesCount].close = 0;
    rootNodes[rootNodesCount].read = 0;
    rootNodes[rootNodesCount].write = 0;
    rootNodes[rootNodesCount].walk = vfs_root_walk;
    rootNodesCount++;

    string_copy(rootNodes[rootNodesCount].name, "..");
    rootNodes[rootNodesCount].inode = 0;
    rootNodes[rootNodesCount].length = 0;
    rootNodes[rootNodesCount].open = 0;
    rootNodes[rootNodesCount].close = 0;
    rootNodes[rootNodesCount].read = 0;
    rootNodes[rootNodesCount].write = 0;
    rootNodes[rootNodesCount].walk = vfs_root_walk;
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

    string_copy(rootNodes[rootNodesCount].name, "initrd");
    rootNodes[rootNodesCount].inode = 0;
    rootNodes[rootNodesCount].length = 0;
    rootNodes[rootNodesCount].open = 0;
    rootNodes[rootNodesCount].close = 0;
    rootNodes[rootNodesCount].read = 0;
    rootNodes[rootNodesCount].write = 0;
    rootNodes[rootNodesCount].walk = initrd_walk;
    rootNodesCount++;

    dev_init();
    initrd_init(location);

    return &root;

}

