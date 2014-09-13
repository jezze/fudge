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
static struct base_device device;
static struct base_keyboard_node node;
static unsigned char buffer[512];
static struct buffer cfifo;
static struct scheduler_rendezvous rdata;
static struct keycode {unsigned char length; unsigned char value[4];} keymap[256];
static unsigned int escaped;
static unsigned int ctrl;
static unsigned int alt;
static unsigned int shift;

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    unsigned char scancode = ps2_getdata(bus);

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

        buffer_wcfifo(&cfifo, keymap[scancode].length, keymap[scancode].value);

    }

    scheduler_rendezvous_unsleep(&rdata);

}

static unsigned int ikeyboard_rdata(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&cfifo, count, buffer);

    scheduler_rendezvous_sleep(&rdata, !count);

    return count;

}

static unsigned int ikeyboard_wdata(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return buffer_wcfifo(&cfifo, count, buffer);

}

static unsigned int ikeyboard_rkeymap(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, keymap, sizeof (struct keycode) * 256, offset);

}

static unsigned int ikeyboard_wkeymap(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_write(keymap, sizeof (struct keycode) * 256, buffer, count, offset);

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PS2_BUS_TYPE)
        return 0;

    return id == PS2_KEYBOARD_DEVICE_TYPE;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    base_initdevice(&device, bus, id);
    base_keyboard_initnode(&node, &device, &ikeyboard);
    base_keyboard_registernode(&node);
    buffer_init(&cfifo, 512, &buffer);
    pic_setroutine(bus, id, handleirq);
    ps2_enable(bus, id);
    ps2_reset(bus, id);
    ps2_disablescanning(bus, id);
    ps2_default(bus, id);
    ps2_identify(bus, id);
    ps2_enablescanning(bus, id);
    ps2_enableinterrupt(bus, id);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    pic_unsetroutine(bus, id);
    base_keyboard_unregisternode(&node);

}

void ps2_keyboard_driver_init()
{

    base_keyboard_initinterface(&ikeyboard, ikeyboard_rdata, ikeyboard_wdata, ikeyboard_rkeymap, ikeyboard_wkeymap);
    base_keyboard_registerinterface(&ikeyboard);
    base_initdriver(&driver, "ps2keyboard", driver_match, driver_attach, driver_detach);
    base_registerdriver(&driver);

}

void ps2_keyboard_driver_destroy()
{

    base_keyboard_unregisterinterface(&ikeyboard);
    base_unregisterdriver(&driver);

}

