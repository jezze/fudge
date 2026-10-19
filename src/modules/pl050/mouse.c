#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/mouse.h>
#include <modules/icp/icp.h>
#include "pl050.h"

#define COMMAND_RATE                    0xF3
#define COMMAND_IDENTIFY                0xF2
#define COMMAND_ENABLESCAN              0xF4
#define COMMAND_DEFAULT                 0xF6
#define COMMAND_RESET                   0xFF

static struct base_driver driver;
static struct mouse_interface mouseinterface;
static unsigned int sequence;
static unsigned int type;
static unsigned char state;
static unsigned char oldstate;
static char relx;
static char rely;
static char relz;

static void notify(void)
{

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

static void handleirq(unsigned int irq)
{

    while (pl050_hasdata(ICP_KMI1))
    {

        unsigned char data = pl050_getdata(ICP_KMI1);

        switch (sequence)
        {

        case 0:
            if (!(data & 0x08))
                continue;

            oldstate = state;
            state = data;
            relz = 0;
            sequence = 1;

            break;

        case 1:
            relx = (state & (1 << 6)) ? 0 : data;
            sequence = 2;

            break;

        case 2:
            rely = (state & (1 << 7)) ? 0 : data;
            sequence = (type) ? 3 : 0;

            break;

        case 3:
            relz = (type == 2) ? ((data & 0x08) ? (char)(data | 0xF0) : (char)(data & 0x0F)) : (char)data;
            sequence = 0;

            break;

        }

        if (!sequence)
            notify();

    }

}

static unsigned char identify(unsigned int id)
{

    return (pl050_command(id, COMMAND_IDENTIFY) == 0xFA) ? pl050_poll(id) : 0;

}

static void rate(unsigned int id, unsigned char value)
{

    if (pl050_command(id, COMMAND_RATE) == 0xFA)
        pl050_command(id, value);

}

static void driver_init(unsigned int id)
{

    mouse_initinterface(&mouseinterface, id);

}

static unsigned int driver_match(unsigned int id)
{

    return id == ICP_KMI1;

}

static void driver_reset(unsigned int id)
{

    pl050_enable(id);
    pl050_command(id, COMMAND_RESET);
    pl050_poll(id);
    pl050_poll(id);
    pl050_command(id, COMMAND_DEFAULT);

    if (identify(id) == 0)
    {

        rate(id, 200);
        rate(id, 100);
        rate(id, 80);

    }

    if (identify(id) == 3)
    {

        type = 1;

        rate(id, 200);
        rate(id, 200);
        rate(id, 80);

    }

    if (identify(id) == 4)
        type = 2;

    pl050_command(id, COMMAND_ENABLESCAN);
    pl050_flush(id);

}

static void driver_attach(unsigned int id)
{

    mouse_registerinterface(&mouseinterface);
    icp_setroutine(icp_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    mouse_unregisterinterface(&mouseinterface);
    icp_unsetroutine(icp_getirq(id), handleirq);
    pl050_disable(id);

}

void module_init(void)
{

    base_initdriver(&driver, "pl050mouse", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, ICP_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, ICP_BUS);

}

