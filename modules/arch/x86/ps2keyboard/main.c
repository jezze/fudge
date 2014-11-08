#include <module.h>
#include <kernel/resource.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>
#include <base/keyboard.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/ps2/ps2.h>

static struct base_driver driver;
static struct base_keyboard_interface keyboardinterface;
static struct base_keyboard_interfacenode keyboardinterfacenode;
static unsigned char buffer[512];
static struct buffer cfifo;
static struct scheduler_rendezvous rdata;

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    unsigned char scancode = ps2_getdata(bus);
    unsigned int count = buffer_wcfifo(&cfifo, 1, &scancode);

    if (count)
        scheduler_rendezvous_unsleep(&rdata);

}

static unsigned int keyboardinterface_rdata(unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&cfifo, count, buffer);

    scheduler_rendezvous_sleep(&rdata, !count);

    return count;

}

static unsigned int keyboardinterface_wdata(unsigned int offset, unsigned int count, void *buffer)
{

    return buffer_wcfifo(&cfifo, count, buffer);

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PS2_BUS_TYPE)
        return 0;

    return id == PS2_KEYBOARD_DEVICE_TYPE;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    base_keyboard_initinterface(&keyboardinterface, bus, id, keyboardinterface_rdata, keyboardinterface_wdata);
    base_keyboard_registerinterface(&keyboardinterface);
    base_keyboard_initinterfacenode(&keyboardinterfacenode, &keyboardinterface);
    base_keyboard_registerinterfacenode(&keyboardinterfacenode);
    buffer_init(&cfifo, 1, 512, &buffer);
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
    base_keyboard_unregisterinterface(&keyboardinterface);
    base_keyboard_unregisterinterfacenode(&keyboardinterfacenode);

}

void init()
{

    base_initdriver(&driver, "ps2keyboard", driver_match, driver_attach, driver_detach);
    base_registerdriver(&driver);

}

void destroy()
{

    base_unregisterdriver(&driver);

}

