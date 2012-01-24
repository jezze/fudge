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

    struct ata_device *device = bus->find_device(bus, ATA_DEVICE_TYPE_ATA, 0);

    if (!device)
        return;

    struct mbr_driver *mbr = (struct mbr_driver *)modules_get_driver(MBR_DRIVER_TYPE);

    if (!mbr)
        return;

    struct mbr_partition *partition = mbr->get_partition(mbr, device, 0);
    unsigned int sectorStart = partition->sectorLba;

    unsigned int blocksize = 1024;
    unsigned int blockstep = blocksize / 512;

    char buffer[1024];

    // Read superblock
    device->read_lba28(device, sectorStart + 1 * blockstep, blockstep, buffer);

    struct ext2_superblock *sb = (struct ext2_superblock *)buffer;

    if (sb->signature != 0xEF53)
        return;

    blocksize = 1024 << sb->blockSize;
    blockstep = blocksize / 512;
    unsigned int nodesize = sb->nodeSize;
    unsigned int firstunreserved = sb->firstUnreservedNode;
    unsigned int nodespergroup = sb->nodeCountGroup;

    log_write("[ext2] Signature: 0x%x\n", sb->signature);
    log_write("[ext2] Version: %d.%d\n", sb->majorVersion, sb->minorVersion);
    log_write("[ext2] Block size: %d\n", blocksize);
    log_write("[ext2] Node size: %d\n", nodesize);
    log_write("[ext2] Blocks per group: %d\n", sb->blockCountGroup);
    log_write("[ext2] Nodes per group: %d\n", sb->nodeCountGroup);
    log_write("[ext2] Superblock index: %d\n", sb->superblockIndex);

    if (sb->majorVersion >= 1)
    {

    }

    firstunreserved += 4;

    // Try to find first unreserved node
    unsigned int nodegroup = get_group(firstunreserved, nodespergroup);
    unsigned int nodeindex = get_index(firstunreserved, nodespergroup);
    unsigned int nodeblock = get_block(firstunreserved, nodesize, blocksize);

    log_write("[ext2] First node: %d\n", firstunreserved);
    log_write("[ext2] First node group: %d\n", nodegroup);
    log_write("[ext2] First node index: %d\n", nodeindex);
    log_write("[ext2] First node block: %d\n", nodeblock);

    // Read first entry in block group descriptor table
    device->read_lba28(device, sectorStart + 2 * blockstep, blockstep, buffer);

    struct ext2_blockgroup *bg = (struct ext2_blockgroup *)buffer;

    //REMEMBER TO OFFSET WITH NODEGROUP. NOW USING ZERO OFFSET

    unsigned int tableblock = bg->blockTableAddress;

    log_write("[ext2] Block address of block usage bitmap: %d\n", bg->blockUsageAddress);
    log_write("[ext2] Block address of node usage bitmap: %d\n", bg->nodeUsageAddress);
    log_write("[ext2] Starting block address of node table: %d\n", tableblock);
    log_write("[ext2] Number of directories: %d\n", bg->directoryCount);

    unsigned int nodesperblock = blocksize / nodesize;

    // Read the node

    unsigned int newindex = nodeindex - nodeblock * nodesperblock;

    log_write("[ext2] New node index: %d\n", newindex);

    device->read_lba28(device, sectorStart + (tableblock + nodeblock) * blockstep, blockstep, buffer);

    void *pb = buffer;

    struct ext2_node *node = pb + nodesize * newindex;
    log_write("[ext2] Node size low: %d\n", node->sizeLow);
    log_write("[ext2] Pointer0: %d\n", node->pointer0);
    log_write("[ext2] Pointer1: %d\n", node->pointer1);
    log_write("[ext2] Pointer2: %d\n", node->pointer2);
    log_write("[ext2] Pointer3: %d\n", node->pointer3);
    log_write("[ext2] Pointer4: %d\n", node->pointer4);
    log_write("[ext2] Pointer5: %d\n", node->pointer5);
    log_write("[ext2] Pointer6: %d\n", node->pointer6);
    log_write("[ext2] Pointer7: %d\n", node->pointer7);
    log_write("[ext2] Pointer8: %d\n", node->pointer8);
    log_write("[ext2] Pointer9: %d\n", node->pointer9);
    log_write("[ext2] Pointer10: %d\n", node->pointer10);
    log_write("[ext2] Pointer11: %d\n", node->pointer11);

    device->read_lba28(device, sectorStart + (node->pointer0) * blockstep, blockstep, buffer);

    log_write("[ext2] Content: %s\n", buffer);

}

void ext2_driver_init(struct ext2_driver *driver)
{

    modules_driver_init(&driver->base, EXT2_DRIVER_TYPE);

    driver->base.start = ext2_driver_start;

}

