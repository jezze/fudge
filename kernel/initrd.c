#include <lib/cbuffer.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/initrd.h>

struct initrd_header *initrdHeader;
struct initrd_file_header *initrdFileHeaders;
struct vfs_node initrdRoot;
struct vfs_node initrdNodes[32];
unsigned int initrdNodesCount;

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

    if (index < initrdNodesCount)
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
    initrdNodes[initrdNodesCount].walk = initrd_walk;
    initrdNodesCount++;

    string_copy(initrdNodes[initrdNodesCount].name, "dev");
    initrdNodes[initrdNodesCount].inode = 0;
    initrdNodes[initrdNodesCount].length = 0;
    initrdNodes[initrdNodesCount].open = 0;
    initrdNodes[initrdNodesCount].close = 0;
    initrdNodes[initrdNodesCount].read = 0;
    initrdNodes[initrdNodesCount].write = 0;
    initrdNodes[initrdNodesCount].walk = dev_walk;
    initrdNodesCount++;

    arch_set_stdin(&initrdNodes[initrdNodesCount]);
    initrdNodesCount++;

    arch_set_stdout(&initrdNodes[initrdNodesCount]);
    initrdNodesCount++;

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

    return &initrdRoot;

}

