#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <mouse/mouse.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/ps2/ps2.h>

static struct base_driver driver;
static struct mouse_interface mouseinterface;

static void handleirq(unsigned int irq, unsigned int id)
{

    unsigned char data = ps2_getdata();

    mouse_notify(&mouseinterface, 1, &data);

}

static void driver_init()
{

    mouse_initinterface(&mouseinterface, &driver);

}

static unsigned int driver_match(unsigned int id)
{

    return id == PS2_MOUSE_DEVICE_TYPE;

}

static void driver_attach(unsigned int id)
{

    ps2_enable(id);
    ps2_reset(id);
    ps2_disablescanning(id);
    ps2_default(id);
    ps2_identify(id);
    ps2_enablescanning(id);
    ps2_enableinterrupt(id);
    mouse_registerinterface(&mouseinterface, id);
    pic_setroutine(ps2_getirq(id), id, handleirq);

}

static void driver_detach(unsigned int id)
{

    mouse_unregisterinterface(&mouseinterface);
    pic_unsetroutine(ps2_getirq(id), id);

}

void module_init()
{

    base_initdriver(&driver, "ps2mouse", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver, PS2_BUS_TYPE);

}

void module_unregister()
{

    base_unregisterdriver(&driver, PS2_BUS_TYPE);

}

