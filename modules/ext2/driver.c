#include <lib/memory.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>
#include <modules/ext2/ext2.h>

static struct ext2_superblock superblock;
static char mem[1024];

static void read_node(unsigned int nodenum, struct ata_device *device, unsigned int sectorstart)
{

    unsigned int blocksize = 1024 << superblock.blockSize;
    unsigned int nodesize = superblock.nodeSize;
    unsigned int sectorsize = blocksize / 512;
    unsigned int nodegroup = (nodenum - 1) / superblock.nodeCountGroup;
    unsigned int nodeindex = (nodenum - 1) % superblock.nodeCountGroup;
    unsigned int nodeblock = (nodenum * nodesize) / blocksize;

    void *buffer = mem;

    device->read_lba28(device, sectorstart + 2 * sectorsize, sectorsize, buffer);

    struct ext2_blockgroup *bg = buffer + nodegroup * sizeof (struct ext2_blockgroup);

    device->read_lba28(device, sectorstart + (bg->blockTableAddress + nodeblock) * sectorsize, sectorsize, buffer);

    struct ext2_node *node = buffer + nodesize * (nodeindex % (blocksize / nodesize));

    if (((node->type & 0xF000) == 0x4000))
    {

        device->read_lba28(device, sectorstart + (node->pointer0) * sectorsize, sectorsize, buffer);

        for (;;)
        {

            struct ext2_directory *directory = buffer;

            if (!directory->length)
                return;

            log_write("%d\t%s\n", directory->node, buffer + 8);

            buffer += directory->size;

        }

    }

    if (((node->type & 0xF000) == 0x8000))
    {

        device->read_lba28(device, sectorstart + (node->pointer0) * sectorsize, sectorsize, buffer);

        log_write("File content:\n");
        log_write("%s\n", buffer);

    }

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

    memory_copy(&superblock, buffer, sizeof (struct ext2_superblock));

    if (superblock.signature != 0xEF53)
        return;

    read_node(2, device, sectorstart);

}

void ext2_driver_init(struct ext2_driver *driver)
{

    modules_driver_init(&driver->base, EXT2_DRIVER_TYPE);

    driver->base.start = ext2_driver_start;

}

