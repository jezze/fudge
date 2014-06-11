#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include <base/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>

static struct base_driver driver;
static struct base_block_interface iblock;

static void handle_irq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

}

static void attach(struct base_bus *bus, struct base_device *device)
{

    struct ide_device *ideDevice = (struct ide_device *)device;
    unsigned short irq = bus->device_irq(bus, ideDevice->slave);

    base_block_init_interface(&iblock, 0, 0);
    base_block_register_interface(&iblock, bus, ideDevice->slave);
    pic_set_routine(irq, bus, ideDevice->slave, handle_irq);

}

static void detach(struct base_bus *bus, struct base_device *device)
{

    struct ide_device *ideDevice = (struct ide_device *)device;
    unsigned short irq = bus->device_irq(bus, ideDevice->slave);

    base_block_unregister_interface(&iblock);
    pic_unset_routine(irq, bus, ideDevice->slave);

}

static unsigned int check(struct base_bus *bus, struct base_device *device)
{

    struct ide_device *ideDevice = (struct ide_device *)device;

    if (device->type != IDE_DEVICE_TYPE)
        return 0;

    return ideDevice->type == IDE_DEVICE_TYPE_ATAPI;

}

void init()
{

    base_init_driver(&driver, "atapi", check, attach, detach);
    base_register_driver(&driver);

}

void destroy()
{

    base_unregister_driver(&driver);

}

