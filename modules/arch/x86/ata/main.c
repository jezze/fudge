#include <module.h>
#include <kernel/resource.h>
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

static unsigned int blockinterface_rdata(unsigned int offset, unsigned int count, void *buffer)
{

    if (offset > 0)
        return 0;

    return ide_rlba28(blockinterface.base.bus, 0, 0, 1, buffer);

}

static unsigned int blockinterface_wdata(unsigned int offset, unsigned int count, void *buffer)
{

    if (offset > 0)
        return 0;

    return ide_wlba28(blockinterface.base.bus, 0, 0, 1, buffer);

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != IDE_BUS_TYPE)
        return 0;

    return id == IDE_DEVICE_TYPE_ATA;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    block_registerinterface(&blockinterface, bus, id);
    pic_setroutine(bus, id, handleirq);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    block_unregisterinterface(&blockinterface);
    pic_unsetroutine(bus, id);

}

void init()
{

    base_initdriver(&driver, "ata", driver_match, driver_attach, driver_detach);
    block_initinterface(&blockinterface, &driver, blockinterface_rdata, blockinterface_wdata);
    base_registerdriver(&driver);

}

void destroy()
{

    base_unregisterdriver(&driver);

}

