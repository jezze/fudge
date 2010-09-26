#include <lib/call.h>
#include <lib/cbuffer.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/dev.h>

struct vfs_node dev;
struct vfs_node *devEntries[32];

struct vfs_node devStdin;
struct vfs_node devStdout;
struct vfs_node devRtc;

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

void dev_init()
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

    struct vfs_node *root = call_vfs_find(".");
    vfs_write(root, 0, 1, &dev);

}

