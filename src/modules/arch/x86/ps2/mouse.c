#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/mouse/mouse.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/apic/apic.h>
#include "ps2.h"

static struct base_driver driver;
static struct mouse_interface mouseinterface;
static unsigned int sequence;
static unsigned int type;
static unsigned char state;
static unsigned char oldstate;
static char relx;
static char rely;
static char relz;

static void handleirq(unsigned int irq)
{

    unsigned char data;
    unsigned char status;

    status = ps2_getstatus();

    if (!(status & 0x20))
        return;

    data = ps2_getdata();

    switch (sequence)
    {

    case 0:
        oldstate = state;

        /* Temp fix for unsync issue */
        if (!(data & 0x08))
            return;

        state = data;
        sequence = 1;

        break;

    case 1:
        if (state & (1 << 6))
            relx = 0;
        else
            relx = data;

        sequence = 2;

        break;

    case 2:
        if (state & (1 << 7))
            rely = 0;
        else
            rely = data;

        sequence = (type) ? 3 : 0;

        break;

    case 3:
        switch (type)
        {

        case 1:
            relz = data;

            break;

        case 2:
            relz = (data & 0x0F);

            break;

        }

        sequence = 0;

        break;

    }

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

    if (relz)
        mouse_notifyscroll(&mouseinterface, relz);

}

static void driver_init(unsigned int id)
{

    mouse_initinterface(&mouseinterface, id);

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

    if (ps2_identify(id) == 0)
    {

        ps2_rate(id, 200);
        ps2_rate(id, 100);
        ps2_rate(id, 80);

    }

    if (ps2_identify(id) == 3)
    {

        type = 1;

        ps2_rate(id, 200);
        ps2_rate(id, 100);
        ps2_rate(id, 80);

    }

    if (ps2_identify(id) == 4)
    {

        type = 2;

    }

    ps2_enable(id);
    ps2_enablescanning(id);
    ps2_enableinterrupt(id);

}

static void driver_attach(unsigned int id)
{

    unsigned short irq = ps2_getirq(id);

    mouse_registerinterface(&mouseinterface);
    pic_setroutine(irq, handleirq);
    apic_setroutine(irq, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = ps2_getirq(id);

    mouse_unregisterinterface(&mouseinterface);
    pic_unsetroutine(irq);
    apic_unsetroutine(irq);

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

