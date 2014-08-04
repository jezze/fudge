#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/block.h>
#include <arch/x86/ide/ide.h>
#include <arch/x86/pic/pic.h>

static struct base_driver driver;
static struct base_block_interface iblock;

static struct instance
{

    struct base_device device;
    struct base_block_node node;

} instances[2];

static struct instance *find_instance(struct base_bus *bus, unsigned int id)
{

    unsigned int i;

    for (i = 0; i < 2; i++)
    {

        struct instance *instance = &instances[i];

        if (instance->device.bus == bus && instance->device.id == id)
            return instance;

    }

    return 0;

}

static unsigned int read_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ide_bus *ideBus = (struct ide_bus *)bus;

    if (offset > 0)
        return 0;

    return ide_bus_read_lba28(ideBus, 0, 0, 1, buffer);

}

static unsigned int write_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ide_bus *ideBus = (struct ide_bus *)bus;

    if (offset > 0)
        return 0;

    return ide_bus_write_lba28(ideBus, 0, 0, 1, buffer);

}

static void handle_irq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    memory_copy((void *)0xB8000, "i n t ", 6);

}

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != IDE_BUS_TYPE)
        return 0;

    return id == IDE_DEVICE_TYPE_ATA;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    struct instance *instance = find_instance(0, 0);

    base_init_device(&instance->device, bus, id);
    base_block_init_node(&instance->node, &instance->device, &iblock);
    base_block_register_node(&instance->node);
    pic_set_routine(bus, id, handle_irq);

}

static void detach(struct base_bus *bus, unsigned int id)
{

    pic_unset_routine(bus, id);

}

void init()
{

    memory_clear(instances, sizeof (struct instance) * 2);
    base_block_init_interface(&iblock, read_data, write_data);
    base_block_register_interface(&iblock);
    base_init_driver(&driver, "ata", check, attach, detach);
    base_register_driver(&driver);

}

void destroy()
{

    base_block_unregister_interface(&iblock);
    base_unregister_driver(&driver);

}

