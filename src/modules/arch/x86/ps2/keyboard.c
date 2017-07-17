#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/keyboard/keyboard.h>
#include <modules/arch/x86/pic/pic.h>
#include "ps2.h"

static struct base_driver driver;
static struct keyboard_interface keyboardinterface;

static void handleirq(unsigned int irq)
{

    unsigned char data;

    if (!ps2_checkdata(PS2_KEYBOARD))
        return;

    data = ps2_getdata();

    keyboard_notify(&keyboardinterface, &data, 1);

    if (data & 0x80)
        keyboard_notifyrelease(&keyboardinterface, data);
    else
        keyboard_notifypress(&keyboardinterface, data);

}

static void driver_init(void)
{

    keyboard_initinterface(&keyboardinterface);

}

static unsigned int driver_match(unsigned int id)
{

    return id == PS2_KEYBOARD;

}

static void driver_attach(unsigned int id)
{

    ps2_disable(id);
    ps2_reset(id);
    ps2_default(id);
    ps2_enable(id);
    ps2_enablescanning(id);
    ps2_enableinterrupt(id);
    keyboard_registerinterface(&keyboardinterface, id);
    pic_setroutine(ps2_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    keyboard_unregisterinterface(&keyboardinterface);
    pic_unsetroutine(ps2_getirq(id));

}

void module_init(void)
{

    base_initdriver(&driver, "ps2keyboard", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PS2_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PS2_BUS);

}

