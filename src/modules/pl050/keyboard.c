#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/keyboard.h>
#include <modules/icp/icp.h>
#include "pl050.h"

#define COMMAND_SCANCODESET             0xF0
#define COMMAND_ENABLESCAN              0xF4
#define COMMAND_RESET                   0xFF

static struct base_driver driver;
static struct keyboard_interface keyboardinterface;
static unsigned int extended;

static void handleirq(unsigned int irq)
{

    while (pl050_hasdata(ICP_KMI0))
    {

        unsigned char data = pl050_getdata(ICP_KMI0);

        if (data == 0x00 || data == 0xFF || data == 0xFA)
            continue;

        if (data == 0xE0)
        {

            extended = 1;

            continue;

        }

        if (data & 0x80)
        {

            if (extended)
                keyboard_notifyrelease(&keyboardinterface, 0xE0);

            keyboard_notifyrelease(&keyboardinterface, data);

        }

        else
        {

            if (extended)
                keyboard_notifypress(&keyboardinterface, 0xE0);

            keyboard_notifypress(&keyboardinterface, data);

        }

        extended = 0;

    }

}

static void driver_init(unsigned int id)
{

    keyboard_initinterface(&keyboardinterface, id);

}

static unsigned int driver_match(unsigned int id)
{

    return id == ICP_KMI0;

}

/* there is no 8042 to translate, so the keyboard is asked for scancode set 1 itself */
static void driver_reset(unsigned int id)
{

    pl050_enable(id);
    pl050_command(id, COMMAND_RESET);
    pl050_poll(id);
    pl050_command(id, COMMAND_SCANCODESET);
    pl050_command(id, 0x01);
    pl050_command(id, COMMAND_ENABLESCAN);
    pl050_flush(id);

}

static void driver_attach(unsigned int id)
{

    keyboard_registerinterface(&keyboardinterface);
    icp_setroutine(icp_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    keyboard_unregisterinterface(&keyboardinterface);
    icp_unsetroutine(icp_getirq(id), handleirq);
    pl050_disable(id);

}

void module_init(void)
{

    base_initdriver(&driver, "pl050keyboard", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, ICP_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, ICP_BUS);

}

