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

    log_write("[ext2] Signature: %x\n", sb->signature);

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

