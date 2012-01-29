#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>
#include <modules/ext2/ext2.h>

static char mem[1024];

static void read_node(unsigned int nodenum, struct ata_device *device, unsigned int blocksize, unsigned int nodesize, unsigned int sectorstart, unsigned int sectorsize, unsigned int nodesperblock)
{

    void *buffer = mem;
    unsigned int nodegroup = (nodenum - 1) / nodesperblock;
    unsigned int nodeindex = (nodenum - 1) % nodesperblock;
    unsigned int nodeblock = (nodenum * nodesize) / blocksize;

    device->read_lba28(device, sectorstart + 2 * sectorsize, sectorsize, buffer);

    struct ext2_blockgroup *bg = buffer + nodegroup * sizeof (struct ext2_blockgroup);

    device->read_lba28(device, sectorstart + (bg->blockTableAddress + nodeblock) * sectorsize, sectorsize, buffer);

    struct ext2_node *node = buffer + nodesize * (nodeindex % (blocksize / nodesize));

    if (!((node->type & 0xF000) == 0x8000))
        return;

    device->read_lba28(device, sectorstart + (node->pointer0) * sectorsize, sectorsize, buffer);

    log_write("Content:\n%s\n", buffer);

}

static void ext2_driver_start(struct modules_driver *self)
{

    // FIX: Not only first ata device
    struct ata_device *device = (struct ata_device *)modules_get_device(ATA_DEVICE_TYPE);

    if (!device)
        return;

    struct mbr_driver *mbr = (struct mbr_driver *)modules_get_driver(MBR_DRIVER_TYPE);

    if (!mbr)
        return;

    // FIX: Not only partition 0
    struct mbr_partition *partition = mbr->get_partition(mbr, device, 0);
    unsigned int sectorstart = partition->sectorLba;

    void *buffer = mem;

    device->read_lba28(device, sectorstart + 1 * 2, 2, buffer);

    struct ext2_superblock *sb = buffer;

    if (sb->signature != 0xEF53)
        return;

    // FIX: If blocksize is different than 1024 the buffer will be to small
    unsigned int blocksize = 1024 << sb->blockSize;
    unsigned int nodesize = sb->nodeSize;
    unsigned int sectorsize = blocksize / 512;
    unsigned int nodesperblock = sb->nodeCountGroup;
    unsigned int first = sb->firstUnreservedNode;
    unsigned int total = sb->nodeCount - sb->nodeCountUnalloc;

    log_write("First: %d\n", first);
    log_write("Total: %d\n", total);

    if (sb->majorVersion >= 1)
    {

    }

    unsigned int i;

    for (i = first; i < first + total; i++)
        read_node(i, device, blocksize, nodesize, sectorstart, sectorsize, nodesperblock);

}

void ext2_driver_init(struct ext2_driver *driver)
{

    modules_driver_init(&driver->base, EXT2_DRIVER_TYPE);

    driver->base.start = ext2_driver_start;

}

