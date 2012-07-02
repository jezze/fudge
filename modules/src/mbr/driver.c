#include <memory.h>
#include <modules.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/ata/ata.h>
#include <mbr/mbr.h>

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

static void attach(struct modules_device *device)
{

    unsigned int i;
    char buffer[512];
    struct ata_device *ataDevice = (struct ata_device *)device;
    struct mbr_driver *driver = (struct mbr_driver *)device->driver;

    apic_register_routine(ataDevice->irq, device, handle_irq);

    ataDevice->read_lba28(ataDevice, 0, 1, buffer);

    for (i = 0; i < MBR_PARTITION_SLOTS; i++)
        driver->add_device(driver, ataDevice, buffer + MBR_PARTITION_OFFSET + i * MBR_PARTITION_SIZE);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    struct ata_device *ataDevice;

    if (device->type != ATA_DEVICE_TYPE)
        return 0;

    ataDevice = (struct ata_device *)device;

    return ataDevice->type == ATA_DEVICE_TYPE_ATA;

}

void mbr_driver_init(struct mbr_driver *driver)
{

    memory_clear(driver, sizeof (struct mbr_driver));

    modules_driver_init(&driver->base, MBR_DRIVER_TYPE, "mbr", 0, check, attach);

    driver->add_device = add_device;
    driver->get_device = get_device;

}

