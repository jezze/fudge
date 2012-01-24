#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>
#include <modules/ext2/ext2.h>

static unsigned int get_group(unsigned int node, unsigned int nodespergroup)
{

    return (node - 1) / nodespergroup;

}

static unsigned int get_index(unsigned int node, unsigned int nodespergroup)
{

    return (node - 1) % nodespergroup;

}

static unsigned int get_block(unsigned int index, unsigned int nodesize, unsigned int blocksize)
{

    return (index * nodesize) / blocksize;

}

static void ext2_driver_start(struct modules_driver *self)
{

    struct ata_bus *bus = (struct ata_bus *)modules_get_bus(ATA_BUS_TYPE);

    if (!bus)
        return;

    // FIX: Not only first ata device

    struct ata_device *device = bus->find_device(bus, ATA_DEVICE_TYPE_ATA, 0);

    if (!device)
        return;

    struct mbr_driver *mbr = (struct mbr_driver *)modules_get_driver(MBR_DRIVER_TYPE);

    if (!mbr)
        return;

    // FIX: Not only partition 0

    struct mbr_partition *partition = mbr->get_partition(mbr, device, 0);
    unsigned int sectorstart = partition->sectorLba;

    char mem[1024];
    void *buffer = mem;

    // Read superblock

    device->read_lba28(device, sectorstart + 1 * 2, 2, buffer);

    struct ext2_superblock *sb = buffer;

    if (sb->signature != 0xEF53)
        return;

    // FIX: If blocksize is different than 1024 the buffer will be to small

    unsigned int blocksize = 1024 << sb->blockSize;
    unsigned int nodesize = sb->nodeSize;
    unsigned int sectorsize = blocksize / 512;

    if (sb->majorVersion >= 1)
    {

    }

    // Try to find forth unreserved node
    // FIX: Select any node
    unsigned int nodenum = sb->firstUnreservedNode + 4;
    unsigned int nodegroup = get_group(nodenum, sb->nodeCountGroup);
    unsigned int nodeindex = get_index(nodenum, sb->nodeCountGroup);
    unsigned int nodeblock = get_block(nodenum, nodesize, blocksize);

    // Read block group descriptor table

    device->read_lba28(device, sectorstart + 2 * sectorsize, sectorsize, buffer);

    struct ext2_blockgroup *bg = buffer + nodegroup * sizeof (struct ext2_blockgroup);

    // Read the node

    device->read_lba28(device, sectorstart + (bg->blockTableAddress + nodeblock) * sectorsize, sectorsize, buffer);

    struct ext2_node *node = buffer + nodesize * (nodeindex % (blocksize / nodesize));

    // Read content

    device->read_lba28(device, sectorstart + (node->pointer0) * sectorsize, sectorsize, buffer);

    log_write(buffer);

}

void ext2_driver_init(struct ext2_driver *driver)
{

    modules_driver_init(&driver->base, EXT2_DRIVER_TYPE);

    driver->base.start = ext2_driver_start;

}

