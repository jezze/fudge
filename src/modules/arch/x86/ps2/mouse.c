#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/mouse/mouse.h>
#include <modules/arch/x86/pic/pic.h>
#include "ps2.h"

static struct base_driver driver;
static struct mouse_interface mouseinterface;
static unsigned int sequence;
static unsigned char state;
static unsigned char oldstate;
static char relx;
static char rely;

static void handleirq(unsigned int irq)
{

    unsigned char data;

    if (!ps2_checkdata(PS2_MOUSE))
        return;

    data = ps2_getdata();

    switch (sequence)
    {

    case 0:
        state = data;
        sequence = 1;

        break;

    case 1:
        relx = data - ((state << 4) & 0x100);
        sequence = 2;

        break;

    case 2:
        rely = data - ((state << 3) & 0x100);
        sequence = 0;

        break;

    }

    mouse_notify(&mouseinterface, &data, 1);

    if (sequence)
        return;

    if (!(oldstate & 1) && (state & 1))
       mouse_notifypress(&mouseinterface, 1);

    if (!(oldstate & 2) && (state & 2))
       mouse_notifypress(&mouseinterface, 2);

    if (!(oldstate & 4) && (state & 4))
       mouse_notifypress(&mouseinterface, 3);

    if ((oldstate & 1) && !(state & 1))
       mouse_notifyrelease(&mouseinterface, 1);

    if ((oldstate & 2) && !(state & 2))
       mouse_notifyrelease(&mouseinterface, 2);

    if ((oldstate & 4) && !(state & 4))
       mouse_notifyrelease(&mouseinterface, 3);

    if (relx || rely)
       mouse_notifymove(&mouseinterface, relx, -rely);

    oldstate = state;

}

static void driver_init(void)
{

    mouse_initinterface(&mouseinterface);

}

static unsigned int driver_match(unsigned int id)
{

    return id == PS2_MOUSE;

}

static void driver_reset(unsigned int id)
{

    ps2_disable(id);
    ps2_reset(id);
    ps2_default(id);
    ps2_enable(id);
    ps2_enablescanning(id);
    ps2_enableinterrupt(id);

}

static void driver_attach(unsigned int id)
{

    mouse_registerinterface(&mouseinterface, id);
    pic_setroutine(ps2_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    mouse_unregisterinterface(&mouseinterface);
    pic_unsetroutine(ps2_getirq(id));

}

void module_init(void)
{

    base_initdriver(&driver, "ps2mouse", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PS2_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PS2_BUS);

}

