#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include <base/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>

static struct base_driver driver;
static struct base_block_interface iblock;

static unsigned int read_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ide_bus *ideBus = (struct ide_bus *)bus;

    if (offset > 0)
        return 0;

    return ide_bus_read_lba28(ideBus, id, 0, 1, buffer);

}

static unsigned int write_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ide_bus *ideBus = (struct ide_bus *)bus;

    if (offset > 0)
        return 0;

    return ide_bus_write_lba28(ideBus, id, 0, 1, buffer);

}

static void handle_irq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

}

static void attach(struct base_bus *bus, struct base_device *device)
{

    struct ide_device *ideDevice = (struct ide_device *)device;
    unsigned short irq = bus->device_irq(bus, ideDevice->slave);

    base_block_init_interface(&iblock, read_data, write_data);
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

    return ideDevice->type == IDE_DEVICE_TYPE_ATA;

}

void init()
{

    base_init_driver(&driver, "ata", check, attach, detach);
    base_register_driver(&driver);

}

void destroy()
{

    base_unregister_driver(&driver);

}

