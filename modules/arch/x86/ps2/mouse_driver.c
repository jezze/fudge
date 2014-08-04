#include <module.h>
#include <kernel/resource.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>
#include <base/mouse.h>
#include <arch/x86/pic/pic.h>
#include "ps2.h"
#include "mouse_driver.h"

static struct base_driver driver;
static struct base_mouse_interface imouse;

static struct instance
{

    struct base_device device;
    struct base_mouse_node node;
    unsigned char buffer[512];
    struct buffer_cfifo cfifo;
    unsigned char cycle;
    struct scheduler_rendezvous rdata;

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

static void reset(struct base_bus *bus)
{

    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xFF);
    ps2_bus_read_data(bus);

}

static void set_defaults(struct base_bus *bus)
{

    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xF6);
    ps2_bus_read_data(bus);

}

static void identify(struct base_bus *bus)
{

    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xF2);
    ps2_bus_read_data(bus);

}

static void enable_scanning(struct base_bus *bus)
{

    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xF4);
    ps2_bus_read_data(bus);

}

static void disable_scanning(struct base_bus *bus)
{

    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xF5);
    ps2_bus_read_data(bus);

}

static unsigned int read_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct instance *instance = find_instance(bus, id);

    count = buffer_read_cfifo(&instance->cfifo, count, buffer);

    scheduler_rendezvous_sleep(&instance->rdata, !count);

    return count;

}

static void handle_irq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    struct instance *instance = find_instance(bus, id);
    unsigned char data = ps2_bus_read_data_async(bus);

    switch (instance->cycle)
    {

    case 0:
        instance->cycle++;

        break;

    case 1:
        instance->cycle++;

        break;

    case 2:
        instance->cycle = 0;

        break;

    }

    buffer_write_cfifo(&instance->cfifo, 1, &data);
    scheduler_rendezvous_unsleep(&instance->rdata);

}

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PS2_BUS_TYPE)
        return 0;

    return id == PS2_MOUSE_DEVICE_TYPE;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    struct instance *instance = find_instance(0, 0);

    base_init_device(&instance->device, bus, id);
    base_mouse_init_node(&instance->node, &instance->device, &imouse);
    base_mouse_register_node(&instance->node);
    buffer_init_cfifo(&instance->cfifo, 512, &instance->buffer);
    pic_set_routine(bus, id, handle_irq);
    ps2_bus_enable_device(bus, id);
    ps2_bus_enable_interrupt(bus, id);
    disable_scanning(bus);
    reset(bus);
    set_defaults(bus);
    identify(bus);
    enable_scanning(bus);

}

static void detach(struct base_bus *bus, unsigned int id)
{

    pic_unset_routine(bus, id);

}

void ps2_mouse_driver_init()
{

    memory_clear(instances, sizeof (struct instance) * 2);
    base_mouse_init_interface(&imouse, read_data);
    base_mouse_register_interface(&imouse);
    base_init_driver(&driver, "ps2mouse", check, attach, detach);
    base_register_driver(&driver);

}

void ps2_mouse_driver_destroy()
{

    base_mouse_unregister_interface(&imouse);
    base_unregister_driver(&driver);

}

