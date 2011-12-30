#include <lib/memory.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

static struct mbr_partition partitions[MBR_PARTITION_SLOTS];

static struct mbr_partition *mbr_driver_get_partition(struct mbr_driver *self, struct ata_device *device, unsigned int index)
{

    char buffer[512];

    device->read_lba28(device, 0, 1, buffer);

    unsigned int i;

    for (i = 0; i < MBR_PARTITION_SLOTS; i++)
        memory_copy(&partitions[i], buffer + MBR_PARTITION_OFFSET + i * MBR_PARTITION_SIZE, sizeof (struct mbr_partition));

    struct mbr_partition *partition = &partitions[index];

    if (!partitions->systemId)
        return 0;

    return partition;

}

static void mbr_driver_start(struct modules_driver *self)
{

}

static void mbr_driver_attach(struct modules_driver *self, struct modules_device *device)
{

    device->driver = self;

}

static unsigned int mbr_driver_check(struct modules_driver *self, struct modules_device *device)
{

    if (device->type != ATA_DEVICE_TYPE)
        return 0;

    struct ata_device *ataDevice = (struct ata_device *)device;

    return ataDevice->type == ATA_DEVICE_TYPE_ATA;

}

void mbr_driver_init(struct mbr_driver *driver)
{

    modules_driver_init(&driver->base, MBR_DRIVER_TYPE);

    driver->base.start = mbr_driver_start;
    driver->base.attach = mbr_driver_attach;
    driver->base.check = mbr_driver_check;
    driver->get_partition = mbr_driver_get_partition;

}

