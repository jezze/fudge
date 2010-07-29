#include <lib/cbuffer.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/initrd.h>

struct initrd_header *initrdHeader;
struct initrd_file_header *initrdFileHeaders;
struct vfs_node initrdRoot;
struct vfs_node initrdNodes[32];

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

    if (index < initrdHeader->nfiles)
        return &initrdNodes[index];
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

    initrdHeader = (struct initrd_header *)location;
    initrdFileHeaders = (struct initrd_file_header *)(location + sizeof (struct initrd_header));

    string_copy(initrdRoot.name, "initrd");
    initrdRoot.inode = 0;
    initrdRoot.length = 0;
    initrdRoot.open = 0;
    initrdRoot.close = 0;
    initrdRoot.read = 0;
    initrdRoot.write = 0;
    initrdRoot.walk = initrd_walk;

    unsigned int i;

    for (i = 0; i < initrdHeader->nfiles; i++)
    {

        initrdFileHeaders[i].offset += location;
        string_copy(initrdNodes[i].name, initrdFileHeaders[i].name);
        initrdNodes[i].inode = i;
        initrdNodes[i].length = initrdFileHeaders[i].length;
        initrdNodes[i].open = 0;
        initrdNodes[i].close = 0;
        initrdNodes[i].read = initrd_read;
        initrdNodes[i].write = 0;
        initrdNodes[i].walk = 0;

    }

    arch_set_stdin(&initrdNodes[initrdHeader->nfiles]);
    initrdHeader->nfiles++;

    arch_set_stdout(&initrdNodes[initrdHeader->nfiles]);
    initrdHeader->nfiles++;


    devNodesCount = 0;

    struct vfs_node *dev = &initrdNodes[initrdHeader->nfiles];

    string_copy(dev->name, "dev");
    dev->inode = 0;
    dev->length = 0;
    dev->open = 0;
    dev->close = 0;
    dev->read = 0;
    dev->write = 0;
    dev->walk = dev_walk;

    initrdHeader->nfiles++;

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

    return &initrdRoot;

}

