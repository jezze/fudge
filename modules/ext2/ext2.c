#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver driver;

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

static void read()
{

    struct ata_bus *bus = (struct ata_bus *)modules_get_bus(ATA_BUS_TYPE, 0);

    if (!bus)
        return;

    struct ata_device *device = bus->find_device(bus, ATA_DEVICE_TYPE_ATA, 0);

    if (!device)
        return;

    struct mbr_driver *mbr = (struct mbr_driver *)modules_get_driver(MBR_DRIVER_TYPE, 0);

    if (!mbr)
        return;

    struct mbr_partition *partition = mbr->get_partition(mbr, 0);
    unsigned int blockstart = partition->sectorLba + 2;

    char buffer[4096];

    device->read_lba28(device, blockstart, 2, buffer);

    struct ext2_superblock *sb = (struct ext2_superblock *)buffer;

    if (sb->signature != 0xEF53)
        return;

    unsigned int blocksize = 1024 << sb->blockSize;
    unsigned int nodesize = sb->nodeSize;
    unsigned int blockstep = blocksize / 512;
    unsigned int firstunreserved = sb->firstUnreservedNode;

    log_write("[ext2] Signature: 0x%x\n", sb->signature);
    log_write("[ext2] Version: %d.%d\n", sb->majorVersion, sb->minorVersion);;
    log_write("[ext2] Block size: %d\n", blocksize);
    log_write("[ext2] Node size: %d\n", nodesize);
    log_write("[ext2] Blocks per group: %d\n", sb->blockCountGroup);
    log_write("[ext2] Nodes per group: %d\n", sb->nodeCountGroup);
    log_write("[ext2] First unreserved node: %d\n", firstunreserved);

    if (sb->majorVersion >= 1)
    {

        log_write("[ext2] Volume: %s\n", sb->volume);
        log_write("[ext2] Last mount: %s\n", sb->lastmount);

    }

    device->read_lba28(device, blockstart + blockstep, blockstep, buffer);

    struct ext2_blockgroup *bg = (struct ext2_blockgroup *)buffer;

    log_write("[ext2] Block block usage bitmap: 0x%x\n", bg->blockUsageAddress);
    log_write("[ext2] Block node usage bitmap: 0x%x\n", bg->nodeUsageAddress);
    log_write("[ext2] Starting block address node table: 0x%x\n", bg->blockTableAddress);
    log_write("[ext2] Number of directories: %d\n", bg->directoryCount);

    device->read_lba28(device, blockstart + blockstep * bg->blockTableAddress, blockstep * 4, buffer);

    unsigned int i;

    for (i = 0; i < 16; i++)
    {

        struct ext2_node *node = (struct ext2_node *)(buffer + i * nodesize);

        unsigned int type = node->type & 0xF000;

        if (type != 0x8000)
            continue;

        log_write("[ext2] Node type: 0x%x Perm: 0x%x Block0: 0x%x\n", node->type & 0xF000, node->type & 0x0FFF, node->pointer0);

        //device->read_lba28(device, blockstart + blockstep * node->pointer0, blockstep, content);
        
//        log_write("[ext2] Content: %s\n", content);

    }

}

void ext2_driver_init(struct ext2_driver *driver)
{

    modules_driver_init(&driver->base, EXT2_DRIVER_TYPE);

}

void init()
{

    ext2_driver_init(&driver);

    read(); // REMOVE

    modules_register_driver(&driver.base);

}

void destroy()
{

}

