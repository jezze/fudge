#include <lib/memory.h>
#include <kernel/modules.h>
#include <modules/mbr/mbr.h>
#include <modules/ext2/ext2.h>

static void read_superblock(struct mbr_device *device, struct ext2_superblock *sb)
{

    char buffer[1024];

    device->read(device, 2, 2, buffer);

    memory_copy(sb, buffer, sizeof (struct ext2_superblock));

}

static void read_blockgroup(struct mbr_device *device, unsigned int id, struct ext2_blockgroup *bg)
{

    char buffer[1024];
    unsigned int blocksize;
    unsigned int sectorsize;
    unsigned int nodegroup;
    struct ext2_superblock sb;

    read_superblock(device, &sb);

    blocksize = 1024 << sb.blockSize;
    sectorsize = blocksize / 512;
    nodegroup = (id - 1) / sb.nodeCountGroup;

    device->read(device, 2 * sectorsize, sectorsize, buffer);

    memory_copy(bg, buffer + nodegroup * sizeof (struct ext2_blockgroup), sizeof (struct ext2_blockgroup));

}

static void read_node(struct mbr_device *device, unsigned int id, struct ext2_blockgroup *bg, struct ext2_node *node)
{

    char buffer[1024];
    unsigned int blocksize;
    unsigned int sectorsize;
    unsigned int nodesize;
    unsigned int nodeindex;
    unsigned int nodeblock;
    struct ext2_superblock sb;

    read_superblock(device, &sb);

    blocksize = 1024 << sb.blockSize;
    sectorsize = blocksize / 512;
    nodesize = sb.nodeSize;
    nodeindex = (id - 1) % sb.nodeCountGroup;
    nodeblock = (id * nodesize) / blocksize;

    device->read(device, (bg->blockTableAddress + nodeblock) * sectorsize, sectorsize, buffer);

    memory_copy(node, buffer + nodesize * (nodeindex % (blocksize / nodesize)), sizeof (struct ext2_node));

}

static void read_content(struct mbr_device *device, struct ext2_node *node, void *buffer)
{

    unsigned int blocksize;
    unsigned int sectorsize;
    struct ext2_superblock sb;

    read_superblock(device, &sb);

    blocksize = 1024 << sb.blockSize;
    sectorsize = blocksize / 512;

    device->read(device, (node->pointer0) * sectorsize, sectorsize, buffer);

}

static void add_filesystem(struct ext2_driver *driver, struct mbr_device *device)
{

    struct ext2_filesystem *filesystem = &driver->filesystems[driver->filesystemsCount];

    ext2_filesystem_init(filesystem, driver, device);
    modules_register_filesystem(&filesystem->base);
    filesystem->base.path = "/hda/";

    driver->filesystemsCount++;

}

static void attach(struct modules_device *device)
{

    struct mbr_device *mbrDevice = (struct mbr_device *)device;
    struct ext2_driver *driver = (struct ext2_driver *)device->driver;

    add_filesystem(driver, mbrDevice);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    struct ext2_superblock sb;
    struct mbr_device *mbrDevice;

    if (device->type != MBR_DEVICE_TYPE)
        return 0;

    mbrDevice = (struct mbr_device *)device;

    read_superblock(mbrDevice, &sb);

    if (sb.signature != 0xEF53)
        return 0;

    return 1;

}

void ext2_driver_init(struct ext2_driver *driver)
{

    memory_clear(driver, sizeof (struct ext2_driver));

    modules_driver_init(&driver->base, EXT2_DRIVER_TYPE, "ext2", 0, check, attach);

    driver->read_blockgroup = read_blockgroup;
    driver->read_node = read_node;
    driver->read_content = read_content;

}

