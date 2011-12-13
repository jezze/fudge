#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/ata/ata.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver driver;

void ext2_driver_init(struct ext2_driver *driver)
{

    struct ata_bus *bus = (struct ata_bus *)modules_get_bus(ATA_BUS_TYPE);

    if (!bus)
        return;

    struct ata_device *device = bus->find_device(bus, ATA_DEVICE_TYPE_ATA);

    if (!device)
        return;

    char buffer[512];

    device->read_lba28(device, 2, 0, buffer);

    struct ext2_superblock *sb = (struct ext2_superblock *)buffer;

    if (sb->signature != 0xEF53)
        return;

    log_write("[ext2] Signature: 0x%x\n", sb->signature);
    log_write("[ext2] Block size: 0x%x\n", sb->blockSize);
    log_write("[ext2] Blocks per group: %d\n", sb->blockCountGroup);
    log_write("[ext2] Nodes per group: %d\n", sb->nodeCountGroup);

    device->read_lba28(device, 3, 0, buffer);

    struct ext2_blockgroup *bg = (struct ext2_blockgroup *)buffer;

    log_write("[ext2] Block usage bitmap address: 0x%x\n", bg->blockUsageAddress);
    log_write("[ext2] Node usage bitmap address: 0x%x\n", bg->nodeUsageAddress);
    log_write("[ext2] Block table address: 0x%x\n", bg->blockTableAddress);

    modules_driver_init(&driver->base, EXT2_DRIVER_TYPE);

}

void init()
{

    ext2_driver_init(&driver);

    modules_register_driver(&driver.base);

}

void destroy()
{

}

