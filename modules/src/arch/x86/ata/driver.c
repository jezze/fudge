#include <memory.h>
#include <modules.h>
#include <block/block.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/ata/ata.h>

static void handle_irq(struct modules_device *device)
{

}

static void attach(struct modules_device *device)
{

    struct ata_device *ataDevice = (struct ata_device *)device;

    apic_register_routine(ataDevice->irq, device, handle_irq);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    struct ata_device *ataDevice;

    if (device->type != ATA_DEVICE_TYPE)
        return 0;

    ataDevice = (struct ata_device *)device;

    return ataDevice->type == ATA_DEVICE_TYPE_ATA;

}

void ata_driver_init(struct ata_driver *driver)
{

    memory_clear(driver, sizeof (struct ata_driver));

    modules_driver_init(&driver->base, ATA_DRIVER_TYPE, "ata", 0, check, attach);

}

