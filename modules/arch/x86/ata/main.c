#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>

static struct base_driver driver;
static struct base_block_interface iblock;
static struct base_device device;
static struct base_block_node node;

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    memory_copy((void *)0xB8000, "i n t ", 6);

}

static unsigned int iblock_rdata(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (offset > 0)
        return 0;

    return ide_rlba28(bus, 0, 0, 1, buffer);

}

static unsigned int iblock_wdata(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (offset > 0)
        return 0;

    return ide_wlba28(bus, 0, 0, 1, buffer);

}

static unsigned int driver_check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != IDE_BUS_TYPE)
        return 0;

    return id == IDE_DEVICE_TYPE_ATA;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    base_initdevice(&device, bus, id);
    base_block_initnode(&node, &device, &iblock);
    base_block_registernode(&node);
    pic_setroutine(bus, id, handleirq);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    pic_unsetroutine(bus, id);
    base_block_unregisternode(&node);

}

void init()
{

    base_block_initinterface(&iblock, iblock_rdata, iblock_wdata);
    base_block_registerinterface(&iblock);
    base_initdriver(&driver, "ata", driver_check, driver_attach, driver_detach);
    base_registerdriver(&driver);

}

void destroy()
{

    base_block_unregisterinterface(&iblock);
    base_unregisterdriver(&driver);

}

