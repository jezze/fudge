#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/mouse/mouse.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/ps2/ps2.h>

static struct base_driver driver;
static struct mouse_interface mouseinterface;
static unsigned int sequence;
static unsigned char state;
static char relx;
static char rely;

static void handleirq(unsigned int irq)
{

    unsigned char data = ps2_getdata();

    mouse_notify(&mouseinterface, 1, &data);

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

        if (state & 1)
            mouse_notifymousepress(&mouseinterface, 1);

        if (state & 2)
            mouse_notifymousepress(&mouseinterface, 2);

        if (state & 4)
            mouse_notifymousepress(&mouseinterface, 3);

        if (relx || rely)
            mouse_notifymousemove(&mouseinterface, relx, rely);

        break;

    }

}

static void driver_init(void)
{

    mouse_initinterface(&mouseinterface);

}

static unsigned int driver_match(unsigned int id)
{

    return id == PS2_MOUSE;

}

static void driver_attach(unsigned int id)
{

    ps2_reset(id);
    ps2_default(id);
    ps2_enable(id);
    ps2_enablescanning(id);
    ps2_enableinterrupt(id);
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

    base_initdriver(&driver, "ps2mouse", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PS2_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PS2_BUS);

}

