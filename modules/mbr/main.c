#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

static struct mbr_driver driver;

static unsigned int check_device(struct modules_module *module)
{

    if (!modules_is_device(module))
        return 0;

    if (((struct modules_device *)module)->type != ATA_DEVICE_TYPE)
        return 0;

    struct ata_device *device = (struct ata_device *)module;

    return device->type == ATA_DEVICE_TYPE_ATA;

}

static void idriver(struct modules_module *module)
{

    struct ata_device *device = (struct ata_device *)module;

    mbr_driver_init(&driver, device);
    modules_register_driver(&driver.base);

}

void init()
{

    modules_foreach(check_device, idriver);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

