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

static unsigned int blockinterface_rdata(unsigned int offset, unsigned int count, void *buffer)
{

    if (offset > 0)
        return 0;

    return ide_rlba28(blockinterface.base.id, 0, 0, 1, buffer);

}

static unsigned int blockinterface_wdata(unsigned int offset, unsigned int count, void *buffer)
{

    if (offset > 0)
        return 0;

    return ide_wlba28(blockinterface.base.id, 0, 0, 1, buffer);

}

static void driver_init()
{

    block_initinterface(&blockinterface, &driver, blockinterface_rdata, blockinterface_wdata);

}

static unsigned int driver_match(unsigned int id)
{

    return id == IDE_DEVICE_TYPE_ATA;

}

static void driver_attach(unsigned int id)
{

    block_registerinterface(&blockinterface, id);
    pic_setroutine(ide_getirq(id), id, handleirq);

}

static void driver_detach(unsigned int id)
{

    block_unregisterinterface(&blockinterface);
    pic_unsetroutine(ide_getirq(id), id);

}

void module_init()
{

    base_initdriver(&driver, "ata", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver, IDE_BUS_TYPE);

}

void module_unregister()
{

    base_unregisterdriver(&driver, IDE_BUS_TYPE);

}

