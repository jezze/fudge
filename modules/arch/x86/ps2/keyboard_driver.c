#include <module.h>
#include <kernel/resource.h>
#include <kernel/scheduler.h>
#include <base/base.h>
#include <base/keyboard.h>
#include <arch/x86/pic/pic.h>
#include "ps2.h"
#include "keyboard_driver.h"

static struct base_driver driver;
static struct base_keyboard_interface ikeyboard;
static unsigned char buffer[512];
static struct buffer_cfifo cfifo;
static struct scheduler_rendezvous rdata;
static struct keycode {unsigned char length; unsigned char value[4];} keymap[256];
static unsigned int escaped;
static unsigned int ctrl;
static unsigned int alt;
static unsigned int shift;

static unsigned int read_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_read_cfifo(&cfifo, count, buffer);

    scheduler_rendezvous_sleep(&rdata, !count);

    return count;

}

static unsigned int write_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return buffer_write_cfifo(&cfifo, count, buffer);

}

static unsigned int read_keymap(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, keymap, 2048, offset);

}

static unsigned int write_keymap(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_write(keymap, 2048, buffer, count, offset);

}

static void handle_irq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    unsigned char scancode = ps2_bus_read_data_async(bus);

    if (escaped)
        escaped = 0;

    if (scancode == 0xE0)
        escaped = 1;

    if (scancode & 0x80)
    {

        scancode &= ~0x80;

        if (scancode == 0x1D)
            ctrl = 0;

        if (scancode == 0x2A)
            shift = 0;

        if (scancode == 0x38)
            alt = 0;

    }

    else
    {

        if (scancode == 0x1D)
            ctrl = 1;

        if (scancode == 0x2A)
            shift = 1;

        if (scancode == 0x38)
            alt = 1;

        if (ctrl)
            scancode = 0;

        if (alt)
            scancode = 0;

        if (shift)
            scancode += 128;

        buffer_write_cfifo(&cfifo, keymap[scancode].length, keymap[scancode].value);

    }

    scheduler_rendezvous_unsleep(&rdata);

}

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PS2_BUS_TYPE)
        return 0;

    return id == PS2_KEYBOARD_DEVICE_TYPE;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    unsigned short irq = bus->device_irq(bus, id);

    base_keyboard_connect_interface(&ikeyboard, bus, id);
    buffer_init_cfifo(&cfifo, 512, &buffer);
    pic_set_routine(irq, bus, id, handle_irq);
    ps2_bus_enable_device(bus, id);
    ps2_bus_enable_interrupt(bus, id);

}

static void detach(struct base_bus *bus, unsigned int id)
{

    unsigned short irq = bus->device_irq(bus, id);

    pic_unset_routine(irq, bus, id);

}

void ps2_keyboard_driver_init()
{

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

