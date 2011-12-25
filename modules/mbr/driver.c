#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

static struct mbr_partition partitions[MBR_PARTITION_SLOTS];

static void read(struct mbr_driver *self)
{

    char buffer[512];

    self->device->read_lba28(self->device, 0, 1, buffer);

    unsigned int i;

    for (i = 0; i < MBR_PARTITION_SLOTS; i++)
        memory_copy(&partitions[i], buffer + MBR_PARTITION_OFFSET + i * MBR_PARTITION_SIZE, sizeof (struct mbr_partition));

}

static struct mbr_partition *mbr_driver_get_partition(struct mbr_driver *self, unsigned int index)
{

    read(self);

    struct mbr_partition *partition = &partitions[index];

    if (!partitions->systemId)
        return 0;

    return partition;

}

void mbr_driver_init(struct mbr_driver *driver, struct ata_device *device)
{

    modules_driver_init(&driver->base, MBR_DRIVER_TYPE);
    driver->device = device;
    driver->get_partition = mbr_driver_get_partition;

}

