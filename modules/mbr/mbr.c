#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

static struct mbr_driver driver;

static void read()
{

    struct ata_bus *bus = (struct ata_bus *)modules_get_bus(ATA_BUS_TYPE);

    if (!bus)
        return;

    struct ata_device *device = bus->find_device(bus, ATA_DEVICE_TYPE_ATA);

    if (!device)
        return;

    char buffer[512];

    device->read_lba48(device, 0, 1, buffer);

    struct mbr_header *header = (struct mbr_header *)buffer;

}

void mbr_driver_init(struct mbr_driver *driver)
{

    modules_driver_init(&driver->base, MBR_DRIVER_TYPE);

}

void init()
{

    mbr_driver_init(&driver);

    read(); // REMOVE

    modules_register_driver(&driver.base);

}

void destroy()
{

}

