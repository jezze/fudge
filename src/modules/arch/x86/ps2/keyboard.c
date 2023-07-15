#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/keyboard/keyboard.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/apic/apic.h>
#include "ps2.h"

static struct base_driver driver;
static struct keyboard_interface keyboardinterface;
static unsigned int extended;

static void handleirq(unsigned int irq)
{

    unsigned char data;
    unsigned char status;

    status = ps2_getstatus();

    if ((status & 0x20))
        return;

    data = ps2_getdata();

    if (data == 0x00)
        return;

    if (data == 0xFF)
        return;

    if (data == 0xE0)
    {

        extended = 1;

        return;

    }

    if (data & 0x80)
    {

        if (extended)
        {

            keyboard_notifyrelease(&keyboardinterface, 0xE0);

            extended = 0;

        }

        keyboard_notifyrelease(&keyboardinterface, data);

    }

    else
    {

        if (extended)
        {

            keyboard_notifypress(&keyboardinterface, 0xE0);

            extended = 0;

        }

        keyboard_notifypress(&keyboardinterface, data);

    }

}

static void driver_init(unsigned int id)
{

    keyboard_initinterface(&keyboardinterface, id);

}

static unsigned int driver_match(unsigned int id)
{

    return id == PS2_KEYBOARD;

}

static void driver_reset(unsigned int id)
{

    ps2_disable(id);
    ps2_reset(id);
    ps2_default(id);
    ps2_enable(id);
    ps2_enablescanning(id);
    ps2_enabletranslation(id);
    ps2_enableinterrupt(id);

}

static void driver_attach(unsigned int id)
{

    unsigned short irq = ps2_getirq(id);

    keyboard_registerinterface(&keyboardinterface);
    pic_setroutine(irq, handleirq);
    apic_setroutine(irq, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = ps2_getirq(id);

    keyboard_unregisterinterface(&keyboardinterface);
    pic_unsetroutine(irq);
    apic_unsetroutine(irq);

}

void module_init(void)
{

    base_initdriver(&driver, "ps2keyboard", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PS2_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PS2_BUS);

}

