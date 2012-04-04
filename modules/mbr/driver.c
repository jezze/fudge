#include <lib/memory.h>
#include <kernel/irq.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

static struct mbr_device *get_device(struct mbr_driver *self, unsigned int index)
{

    struct mbr_device *device = &self->devices[index];

    if (!device->partition.systemId)
        return 0;

    return device;

}

static void handle_irq(struct modules_device *device)
{

}

static void add_device(struct mbr_driver *self, struct ata_device *ataDevice, void *buffer)
{

    struct mbr_device *device = &self->devices[self->devicesCount];

    mbr_device_init(device, ataDevice, buffer);
    modules_register_device(&device->base);

    self->devicesCount++;

}

static void attach(struct modules_driver *self, struct modules_device *device)
{

    struct mbr_driver *driver = (struct mbr_driver *)self;
    struct ata_device *ataDevice = (struct ata_device *)device;

    irq_register_routine(ataDevice->irq, device, handle_irq);

    char buffer[512];

    ataDevice->read_lba28(ataDevice, 0, 1, buffer);

    unsigned int i;

    for (i = 0; i < MBR_PARTITION_SLOTS; i++)
        driver->add_device(driver, ataDevice, buffer + MBR_PARTITION_OFFSET + i * MBR_PARTITION_SIZE);

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

    memory_clear(driver, sizeof (struct mbr_driver));

    modules_driver_init(&driver->base, MBR_DRIVER_TYPE, "mbr");

    driver->base.check = check;
    driver->base.attach = attach;
    driver->add_device = add_device;
    driver->get_device = get_device;

}

