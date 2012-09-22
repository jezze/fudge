#include <memory.h>
#include <vfs.h>
#include <base/base.h>
#include <block/block.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/ata/ata.h>

static void handle_irq(struct base_device *device)
{

}

static void attach(struct base_device *device)
{

    struct ata_device *ataDevice = (struct ata_device *)device;

    apic_register_routine(ataDevice->irq, device, handle_irq);

}

static unsigned int check(struct base_driver *self, struct base_device *device)
{

    struct ata_device *ataDevice;

    if (device->type != ATA_DEVICE_TYPE)
        return 0;

    ataDevice = (struct ata_device *)device;

    return ataDevice->type == ATA_DEVICE_TYPE_ATA;

}

void ata_init_driver(struct ata_driver *driver)
{

    memory_clear(driver, sizeof (struct ata_driver));

    base_init_driver(&driver->base, ATA_DRIVER_TYPE, "ata", 0, check, attach);

}

