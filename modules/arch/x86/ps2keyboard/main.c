#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <keyboard/keyboard.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/ps2/ps2.h>

static struct base_driver driver;
static struct keyboard_interface keyboardinterface;

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    unsigned char data = ps2_getdata(bus);

    keyboard_notify(1, &data);

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PS2_BUS_TYPE)
        return 0;

    return id == PS2_KEYBOARD_DEVICE_TYPE;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    keyboard_registerinterface(&keyboardinterface, bus, id);
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

    keyboard_unregisterinterface(&keyboardinterface);
    pic_unsetroutine(bus, id);

}

void init()
{

    base_initdriver(&driver, "ps2keyboard", driver_match, driver_attach, driver_detach);
    keyboard_initinterface(&keyboardinterface, &driver);
    base_registerdriver(&driver);

}

void destroy()
{

    base_unregisterdriver(&driver);

}

