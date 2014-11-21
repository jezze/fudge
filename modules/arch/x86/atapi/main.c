#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <block/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>

static struct base_driver driver;
static struct block_interface blockinterface;
static struct block_interfacenode blockinterfacenode;

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != IDE_BUS_TYPE)
        return 0;

    return id == IDE_DEVICE_TYPE_ATAPI;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    block_initinterface(&blockinterface, bus, id, 0, 0);
    block_registerinterface(&blockinterface);
    block_initinterfacenode(&blockinterfacenode, &blockinterface);
    block_registerinterfacenode(&blockinterfacenode);
    pic_setroutine(bus, id, handleirq);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    pic_unsetroutine(bus, id);
    block_unregisterinterface(&blockinterface);
    block_unregisterinterfacenode(&blockinterfacenode);

}

void init()
{

    base_initdriver(&driver, "atapi", driver_match, driver_attach, driver_detach);
    base_registerdriver(&driver);

}

void destroy()
{

    base_unregisterdriver(&driver);

}

