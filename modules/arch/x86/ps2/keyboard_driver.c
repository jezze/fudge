#include <module.h>
#include <kernel/resource.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>
#include <base/keyboard.h>
#include <arch/x86/pic/pic.h>
#include "ps2.h"
#include "keyboard_driver.h"

static struct base_driver driver;
static struct base_keyboard_interface ikeyboard;

static struct instance
{

    struct base_device device;
    struct base_keyboard_node node;
    unsigned char buffer[512];
    struct buffer_cfifo cfifo;
    struct scheduler_rendezvous rdata;
    struct keycode {unsigned char length; unsigned char value[4];} keymap[256];
    unsigned int escaped;
    unsigned int ctrl;
    unsigned int alt;
    unsigned int shift;

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

    struct instance *instance = find_instance(bus, id);

    count = buffer_read_cfifo(&instance->cfifo, count, buffer);

    scheduler_rendezvous_sleep(&instance->rdata, !count);

    return count;

}

static unsigned int write_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct instance *instance = find_instance(bus, id);

    return buffer_write_cfifo(&instance->cfifo, count, buffer);

}

static unsigned int read_keymap(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct instance *instance = find_instance(bus, id);

    return memory_read(buffer, count, instance->keymap, 2048, offset);

}

static unsigned int write_keymap(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct instance *instance = find_instance(bus, id);

    return memory_write(instance->keymap, 2048, buffer, count, offset);

}

static void handle_irq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    struct instance *instance = find_instance(bus, id);
    unsigned char scancode = ps2_bus_read_data_async(bus);

    if (instance->escaped)
        instance->escaped = 0;

    if (scancode == 0xE0)
        instance->escaped = 1;

    if (scancode & 0x80)
    {

        scancode &= ~0x80;

        if (scancode == 0x1D)
            instance->ctrl = 0;

        if (scancode == 0x2A)
            instance->shift = 0;

        if (scancode == 0x38)
            instance->alt = 0;

    }

    else
    {

        if (scancode == 0x1D)
            instance->ctrl = 1;

        if (scancode == 0x2A)
            instance->shift = 1;

        if (scancode == 0x38)
            instance->alt = 1;

        if (instance->ctrl)
            scancode = 0;

        if (instance->alt)
            scancode = 0;

        if (instance->shift)
            scancode += 128;

        buffer_write_cfifo(&instance->cfifo, instance->keymap[scancode].length, instance->keymap[scancode].value);

    }

    scheduler_rendezvous_unsleep(&instance->rdata);

}

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PS2_BUS_TYPE)
        return 0;

    return id == PS2_KEYBOARD_DEVICE_TYPE;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    struct instance *instance = find_instance(0, 0);

    base_init_device(&instance->device, bus, id);
    base_keyboard_init_node(&instance->node, &instance->device, &ikeyboard);
    base_keyboard_register_node(&instance->node);
    buffer_init_cfifo(&instance->cfifo, 512, &instance->buffer);
    pic_set_routine(bus, id, handle_irq);
    ps2_bus_enable_device(bus, id);
    ps2_bus_enable_interrupt(bus, id);

}

static void detach(struct base_bus *bus, unsigned int id)
{

    pic_unset_routine(bus, id);

}

void ps2_keyboard_driver_init()
{

    memory_clear(instances, sizeof (struct instance) * 2);
    base_keyboard_init_interface(&ikeyboard, read_data, write_data, read_keymap, write_keymap);
    base_keyboard_register_interface(&ikeyboard);
    base_init_driver(&driver, "ps2keyboard", check, attach, detach);
    base_register_driver(&driver);

}

void ps2_keyboard_driver_destroy()
{

    base_keyboard_unregister_interface(&ikeyboard);
    base_unregister_driver(&driver);

}

