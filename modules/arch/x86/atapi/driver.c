#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <base/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>
#include "atapi.h"

static void handle_irq(struct base_device *device)
{

}

static void attach(struct base_device *device)
{

    struct atapi_driver *driver = (struct atapi_driver *)device->driver;

    base_register_block(&driver->iblock, device);
    pic_set_routine(device, handle_irq);

}

static unsigned int check(struct base_device *device)
{

    struct ide_device *ideDevice = (struct ide_device *)device;

    if (device->type != IDE_DEVICE_TYPE)
        return 0;

    return ideDevice->type == IDE_DEVICE_TYPE_ATAPI;

}

void atapi_init_driver(struct atapi_driver *driver)
{

    memory_clear(driver, sizeof (struct atapi_driver));
    base_init_driver(&driver->base, "atapi", check, attach);
    base_init_block(&driver->iblock, 0, 0);

}

