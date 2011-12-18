#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver driver;

static void read()
{

    struct ata_bus *bus = (struct ata_bus *)modules_get_bus(ATA_BUS_TYPE);

    if (!bus)
        return;

    struct ata_device *device = bus->find_device(bus, ATA_DEVICE_TYPE_ATA);

    if (!device)
        return;

    struct mbr_driver *mbr = (struct mbr_driver *)modules_get_driver(MBR_DRIVER_TYPE);

    if (!mbr)
        return;

    struct mbr_partition *partition = mbr->get_partition(0);

    char buffer[512];

    device->read_lba28(device, partition->sectorLba + 2, 1, buffer);

    struct ext2_superblock *sb = (struct ext2_superblock *)buffer;

    if (sb->signature != 0xEF53)
        return;

    log_write("[ext2] Signature: 0x%x\n", sb->signature);
    log_write("[ext2] Version: %d:%d\n", sb->majorVersion, sb->minorVersion);;
    log_write("[ext2] Block size: %d\n", 1024 << sb->blockSize);
    log_write("[ext2] Blocks per group: %d\n", sb->blockCountGroup);
    log_write("[ext2] Nodes per group: %d\n", sb->nodeCountGroup);

    if (sb->majorVersion >= 1)
    {

        log_write("[ext2] Volume: %s\n", sb->volume);
        log_write("[ext2] Last mount: %s\n", sb->lastmount);

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

