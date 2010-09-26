#include <lib/cbuffer.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/vfs.h>

struct vfs_node root;
struct vfs_node *rootEntries[32];

struct vfs_node dev;
struct vfs_node *devEntries[32];

struct vfs_node devStdin;
struct vfs_node devStdout;
struct vfs_node devRtc;

struct vfs_node initrd;
struct vfs_node *initrdEntries[32];

struct initrd_header *initrdHeader;
struct initrd_file_header *initrdFileHeaders;

struct vfs_node initrdNodes[32];
unsigned int initrdNodesCount;

static struct vfs_node *vfs_root_walk(struct vfs_node *node, unsigned int index)
{

    if (index < root.length)
        return rootEntries[index];
    else
        return 0;

}

static unsigned int vfs_root_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    rootEntries[root.length] = (struct vfs_node *)buffer;
    root.length++;

    return count;

}

static struct vfs_node *dev_walk(struct vfs_node *node, unsigned int index)
{

    if (index < dev.length)
        return devEntries[index];
    else
        return 0;

}

static unsigned int dev_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    devEntries[dev.length] = (struct vfs_node *)buffer;
    dev.length++;

    return count;

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

static struct vfs_node *dev_init()
{

    string_copy(dev.name, "dev");
    dev.inode = 0;
    dev.length = 0;
    dev.open = 0;
    dev.close = 0;
    dev.read = 0;
    dev.write = dev_write;
    dev.walk = dev_walk;

    string_copy(devStdin.name, "stdin");
    devStdin.inode = 0;
    devStdin.length = 0;
    devStdin.open = 0;
    devStdin.close = 0;
    devStdin.read = 0;
    devStdin.write = 0;
    devStdin.walk = 0;

    string_copy(devStdout.name, "stdout");
    devStdout.inode = 0;
    devStdout.length = 0;
    devStdout.open = 0;
    devStdout.close = 0;
    devStdout.read = 0;
    devStdout.write = 0;
    devStdout.walk = 0;

    string_copy(devRtc.name, "rtc");
    devRtc.inode = 0;
    devRtc.length = 0;
    devRtc.open = 0;
    devRtc.close = 0;
    devRtc.read = 0;
    devRtc.write = 0;
    devRtc.walk = 0;

    vfs_write(&dev, 0, 1, &devStdin);
    vfs_write(&dev, 0, 1, &devStdout);
    vfs_write(&dev, 0, 1, &devRtc);

    return &dev;

}

static struct vfs_node *initrd_init(unsigned int location)
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

    return &initrd;

}

struct vfs_node *vfs_init(unsigned int location)
{

    string_copy(root.name, "root");
    root.inode = 0;
    root.length = 0;
    root.open = 0;
    root.close = 0;
    root.read = 0;
    root.write = vfs_root_write;
    root.walk = vfs_root_walk;

    vfs_write(&root, 0, 1, dev_init());
    vfs_write(&root, 0, 1, initrd_init(location));

    return &root;

}

