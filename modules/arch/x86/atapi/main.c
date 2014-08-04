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

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != IDE_BUS_TYPE)
        return 0;

    return id == IDE_DEVICE_TYPE_ATAPI;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    base_block_connect_interface(&iblock.base, bus, id);
    pic_set_routine(bus, id, handle_irq);

}

static void detach(struct base_bus *bus, unsigned int id)
{

    pic_unset_routine(bus, id);

}

void init()
{

    base_block_init_interface(&iblock, 0, 0);
    base_block_register_interface(&iblock);
    base_init_driver(&driver, "atapi", check, attach, detach);
    base_register_driver(&driver);

}

void destroy()
{

    base_block_unregister_interface(&iblock);
    base_unregister_driver(&driver);

}

