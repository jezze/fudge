#include <module.h>
#include <base/base.h>
#include <base/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>

static struct base_driver driver;
static struct base_block_interface iblock;

static void handle_irq(struct base_device *device)
{

}

static void attach(struct base_device *device)
{

    base_block_init_interface(&iblock, 0, 0);
    base_block_register_interface(&iblock, device);
    pic_set_routine(device, handle_irq);

}

static unsigned int check(struct base_device *device)
{

    struct ide_device *ideDevice = (struct ide_device *)device;

    if (device->type != IDE_DEVICE_TYPE)
        return 0;

    return ideDevice->type == IDE_DEVICE_TYPE_ATAPI;

}

void init()
{

    base_init_driver(&driver, "atapi", check, attach);
    base_register_driver(&driver);

}

void destroy()
{

    base_unregister_driver(&driver);

}

