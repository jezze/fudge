#include <module.h>
#include <base/base.h>
#include <base/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>
#include "driver.h"

static void handle_irq(struct base_device *device)
{

}

static unsigned int read_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct ide_device *ideDevice = (struct ide_device *)device;

    if (offset > 0)
        return 0;

    return ide_device_read_lba28(ideDevice, 0, 1, buffer);

}

static unsigned int write_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct ide_device *ideDevice = (struct ide_device *)device;

    if (offset > 0)
        return 0;

    return ide_device_write_lba28(ideDevice, 0, 1, buffer);

}

static void attach(struct base_device *device)
{

    struct ata_driver *driver = (struct ata_driver *)device->driver;

    base_block_register_interface(&driver->iblock, device);
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
    base_block_init_interface(&driver->iblock, read_data, write_data);

}

