#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <mouse/mouse.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/ps2/ps2.h>

static struct base_driver driver;
static struct mouse_interface mouseinterface;

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    unsigned char data = ps2_getdata(bus);

    mouse_notify(1, &data);

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PS2_BUS_TYPE)
        return 0;

    return id == PS2_MOUSE_DEVICE_TYPE;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    ps2_enable(bus, id);
    ps2_reset(bus, id);
    ps2_disablescanning(bus, id);
    ps2_default(bus, id);
    ps2_identify(bus, id);
    ps2_enablescanning(bus, id);
    ps2_enableinterrupt(bus, id);
    mouse_registerinterface(&mouseinterface, bus, id);
    pic_setroutine(bus, id, handleirq);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    mouse_unregisterinterface(&mouseinterface);
    pic_unsetroutine(bus, id);

}

void init()
{

    base_initdriver(&driver, "ps2mouse", driver_match, driver_attach, driver_detach);
    mouse_initinterface(&mouseinterface, &driver);
    base_registerdriver(&driver);

}

void destroy()
{

    base_unregisterdriver(&driver);

}

