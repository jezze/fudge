#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

static struct mbr_driver driver;

void init()
{

    struct ata_bus *bus = (struct ata_bus *)modules_get_bus(ATA_BUS_TYPE, 0);

    if (!bus)
        return;

    struct ata_device *device = bus->find_device(bus, ATA_DEVICE_TYPE_ATA, 0);

    if (!device)
        return;

    mbr_driver_init(&driver, device);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

