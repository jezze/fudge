#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <block/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>

static struct base_driver driver;
static struct block_interface blockinterface;

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

}

static void driver_init(struct base_driver *self)
{

    block_initinterface(&blockinterface, self, 0, 0);

}

static unsigned int driver_match(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    if (bus->type != IDE_BUS_TYPE)
        return 0;

    return id == IDE_DEVICE_TYPE_ATAPI;

}

static void driver_attach(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    block_registerinterface(&blockinterface, bus, id);
    pic_setroutine(bus, id, handleirq);

}

static void driver_detach(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    block_unregisterinterface(&blockinterface);
    pic_unsetroutine(bus, id);

}

void init()
{

    base_initdriver(&driver, "atapi", driver_init, driver_match, driver_attach, driver_detach);
    base_registerdriver(&driver);

}

void destroy()
{

    base_unregisterdriver(&driver);

}

