#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

static struct mbr_driver driver;

static void read_partition(void *buffer, unsigned int offset)
{

    struct mbr_partition *header = (struct mbr_header *)(buffer + offset);

    if (!header->systemId)
        return;

    log_write("[mbr] Partition offset: 0x%x\n", offset);
    log_write("[mbr]   Boot: 0x%x\n", header->boot);
    log_write("[mbr]   Start: %d\n", header->sectorRelative);

    unsigned int c = header->cylinderStart & 0xC0;
    unsigned int h = header->headStart;
    unsigned int s = header->cylinderStart & 0x3F;

    log_write("[mbr]   CHS: %d:%d:%d\n", c, h, s);

}

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

    read_partition(buffer, 0x1BE);
    read_partition(buffer, 0x1CE);
    read_partition(buffer, 0x1DE);
    read_partition(buffer, 0x1EE);

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

