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

    base_block_init_interface(&iblock, read_data, write_data);
    base_block_register_interface(&iblock, device);
    pic_set_routine(device, handle_irq);

}

static unsigned int check(struct base_device *device)
{

    struct ide_device *ideDevice = (struct ide_device *)device;

    if (device->type != IDE_DEVICE_TYPE)
        return 0;

    return ideDevice->type == IDE_DEVICE_TYPE_ATA;

}

void init()
{

    base_init_driver(&driver, "ata", check, attach);
    base_register_driver(&driver);

}

void destroy()
{

    base_unregister_driver(&driver);

}

