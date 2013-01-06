#include <fudge/memory.h>
#include <system/system.h>
#include <base/base.h>
#include <block/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>
#include "ata.h"

static void handle_irq(struct base_device *device)
{

}

static void attach(struct base_device *device)
{

    struct ide_device *ideDevice = (struct ide_device *)device;

    pic_set_routine(ideDevice->irq, device, handle_irq);

}

static unsigned int check(struct base_device *device)
{

    struct ide_device *ideDevice;

    if (device->type != IDE_DEVICE_TYPE)
        return 0;

    ideDevice = (struct ide_device *)device;

    return ideDevice->type == IDE_DEVICE_TYPE_ATA;

}

void ata_init_driver(struct ata_driver *driver)
{

    memory_clear(driver, sizeof (struct ata_driver));

    base_init_driver(&driver->base, ATA_DRIVER_TYPE, "ata", 0, check, attach);
    block_init_interface(&driver->interface, &driver->base, 0, 0);

}

