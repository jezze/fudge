#include <kernel/modules.h>
#include <modules/ata/ata.h>

static struct ata_bus primary;
static struct ata_bus secondary;
static struct ata_device devices[8];
static unsigned int devicesCount;

void add_device(struct ata_bus *bus, unsigned int master, unsigned int type, void *buffer)
{

    struct ata_device *device = &devices[devicesCount];

    ata_device_init(device, bus, master, type);

    if (type == ATA_DEVICE_TYPE_ATA)
        device->configure_ata(device, buffer);

    if (type == ATA_DEVICE_TYPE_ATAPI)
        device->configure_atapi(device, buffer);

    modules_register_device(&device->base);

    devicesCount++;

}

void init()
{

    devicesCount = 0;

    ata_bus_init(&primary, ATA_MASTER_PRIMARY_CONTROL, ATA_MASTER_PRIMARY_DATA, 0x0E);
    ata_bus_init(&secondary, ATA_MASTER_SECONDARY_CONTROL, ATA_MASTER_SECONDARY_DATA, 0x0F);

    modules_register_bus(&primary.base);
    modules_register_bus(&secondary.base);

}

void destroy()
{

    modules_unregister_bus(&primary.base);
    modules_unregister_bus(&secondary.base);

}

