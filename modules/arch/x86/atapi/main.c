#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <block/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>

static struct base_driver driver;
static struct block_interface blockinterface;

static void handleirq(unsigned int irq, unsigned int id)
{

}

static void driver_init()
{

    block_initinterface(&blockinterface, &driver, 0, 0);

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    if (type != IDE_BUS_TYPE)
        return 0;

    return id == IDE_DEVICE_TYPE_ATAPI;

}

static void driver_attach(unsigned int id)
{

    unsigned short irq = ide_getirq(id);

    block_registerinterface(&blockinterface, id);
    pic_setroutine(irq, id, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = ide_getirq(id);

    block_unregisterinterface(&blockinterface);
    pic_unsetroutine(irq, id);

}

void module_init()
{

    base_initdriver(&driver, "atapi", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver);

}

void module_unregister()
{

    base_unregisterdriver(&driver);

}

