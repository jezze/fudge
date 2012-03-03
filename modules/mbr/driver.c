#include <lib/memory.h>
#include <kernel/irq.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

static struct mbr_partition partitions[MBR_PARTITION_SLOTS];

static struct mbr_partition *get_partition(struct mbr_driver *self, struct ata_device *device, unsigned int index)
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

static void start(struct modules_driver *self)
{

}

static void handle_irq_primary(struct modules_device *device)
{

    log_write("[mbr] Primary interrupt\n");    

}

static void handle_irq_secondary(struct modules_device *device)
{

    log_write("[mbr] Secondary interrupt\n");    

}

static void attach(struct modules_driver *self, struct modules_device *device)
{

    irq_register_routine(0x0E, device, handle_irq_primary);
    irq_register_routine(0x0F, device, handle_irq_secondary);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    if (device->type != ATA_DEVICE_TYPE)
        return 0;

    struct ata_device *ataDevice = (struct ata_device *)device;

    return ataDevice->type == ATA_DEVICE_TYPE_ATA;

}

void mbr_driver_init(struct mbr_driver *driver)
{

    modules_driver_init(&driver->base, MBR_DRIVER_TYPE, "mbr");

    driver->base.start = start;
    driver->base.check = check;
    driver->base.attach = attach;
    driver->get_partition = get_partition;

}

