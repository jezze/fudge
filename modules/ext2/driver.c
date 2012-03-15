#include <lib/memory.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>
#include <modules/ext2/ext2.h>

static struct ext2_superblock superblock;

static void read_blockgroup(struct ext2_driver *self, unsigned int id, struct ext2_blockgroup *bg)
{

    char mem[1024];
    void *buffer = mem;

    struct mbr_partition *partition = self->mbrDriver->get_partition(self->mbrDriver, self->ataDevice, 0);
    unsigned int sectorstart = partition->sectorLba;

    unsigned int blocksize = 1024 << superblock.blockSize;
    unsigned int sectorsize = blocksize / 512;

    unsigned int nodegroup = (id - 1) / superblock.nodeCountGroup;

    self->ataDevice->read_lba28(self->ataDevice, sectorstart + 2 * sectorsize, sectorsize, buffer);

    memory_copy(bg, buffer + nodegroup * sizeof (struct ext2_blockgroup), sizeof (struct ext2_blockgroup));

}

static void read_node(struct ext2_driver *self, unsigned int id, struct ext2_blockgroup *bg, struct ext2_node *node)
{

    char mem[1024];
    void *buffer = mem;

    struct mbr_partition *partition = self->mbrDriver->get_partition(self->mbrDriver, self->ataDevice, 0);
    unsigned int sectorstart = partition->sectorLba;

    unsigned int blocksize = 1024 << superblock.blockSize;
    unsigned int sectorsize = blocksize / 512;

    unsigned int nodesize = superblock.nodeSize;
    unsigned int nodeindex = (id - 1) % superblock.nodeCountGroup;
    unsigned int nodeblock = (id * nodesize) / blocksize;

    self->ataDevice->read_lba28(self->ataDevice, sectorstart + (bg->blockTableAddress + nodeblock) * sectorsize, sectorsize, buffer);

    memory_copy(node, buffer + nodesize * (nodeindex % (blocksize / nodesize)), sizeof (struct ext2_node));

}

static void read_content(struct ext2_driver *self, unsigned int id, struct ext2_node *node, void *buffer)
{

    struct mbr_partition *partition = self->mbrDriver->get_partition(self->mbrDriver, self->ataDevice, 0);
    unsigned int sectorstart = partition->sectorLba;

    unsigned int blocksize = 1024 << superblock.blockSize;
    unsigned int sectorsize = blocksize / 512;

    self->ataDevice->read_lba28(self->ataDevice, sectorstart + (node->pointer0) * sectorsize, sectorsize, buffer);

}

static void start(struct modules_driver *self)
{

    struct ext2_driver *driver = (struct ext2_driver *)self;

    driver->ataDevice = (struct ata_device *)modules_get_device(ATA_DEVICE_TYPE);

    if (!driver->ataDevice)
        return;

    driver->mbrDriver = (struct mbr_driver *)modules_get_driver(MBR_DRIVER_TYPE);

    if (!driver->mbrDriver)
        return;

    struct mbr_partition *partition = driver->mbrDriver->get_partition(driver->mbrDriver, driver->ataDevice, 0);
    unsigned int sectorstart = partition->sectorLba;

    char mem[1024];
    void *buffer = &mem;

    driver->ataDevice->read_lba28(driver->ataDevice, sectorstart + 1 * 2, 2, buffer);

    memory_copy(&superblock, buffer, sizeof (struct ext2_superblock));

    if (superblock.signature != 0xEF53)
        return;

}

void ext2_driver_init(struct ext2_driver *driver)
{

    memory_clear(driver, sizeof (struct ext2_driver));

    modules_driver_init(&driver->base, EXT2_DRIVER_TYPE, "ext2");

    driver->base.start = start;
    driver->read_blockgroup = read_blockgroup;
    driver->read_node = read_node;
    driver->read_content = read_content;

}

