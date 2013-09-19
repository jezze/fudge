#include <fudge/module.h>
#include <base/base.h>
#include <base/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>
#include "ata.h"

static void handle_irq(struct base_device *device)
{

}

static void attach(struct base_device *device)
{

    struct ata_driver *driver = (struct ata_driver *)device->driver;

    base_register_block(&driver->iblock, device);
    pic_set_routine(device, handle_irq);

}

static unsigned int check(struct base_device *device)
{

    struct ide_device *ideDevice = (struct ide_device *)device;

    if (device->type != IDE_DEVICE_TYPE)
        return 0;

    return ideDevice->type == IDE_DEVICE_TYPE_ATA;

}

void ata_init_driver(struct ata_driver *driver)
{

    memory_clear(driver, sizeof (struct ata_driver));
    base_init_driver(&driver->base, "ata", check, attach);
    base_init_block(&driver->iblock, 0, 0);

}

