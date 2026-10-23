#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/mouse.h>
#include "amiga.h"

#define REG_CUSTOM                      0x00DFF000
#define REG_JOY0DAT                     0x00A
#define REG_POTGOR                      0x016
#define REG_POTGOR_RIGHT                0x0400
#define REG_POTGOR_MIDDLE               0x0100
#define REG_POTGO                       0x034
#define REG_POTGO_OUTPUT                0xFF00
#define REG_CIAA                        0x00BFE000
#define REG_PRA                         0x001
#define REG_PRA_LEFT                    0x40

static struct base_driver driver;
static struct mouse_interface mouseinterface;
static volatile unsigned short *custom = (volatile unsigned short *)REG_CUSTOM;
static volatile unsigned char *cia = (volatile unsigned char *)REG_CIAA;
static unsigned short position;
static unsigned int pressed;

static void updatebutton(unsigned int button, unsigned int down)
{

    unsigned int bit = 1U << button;

    if (down && !(pressed & bit))
    {

        pressed |= bit;

        mouse_notifypress(&mouseinterface, button);

    }

    else if (!down && (pressed & bit))
    {

        pressed &= ~bit;

        mouse_notifyrelease(&mouseinterface, button);

    }

}

/* joy0dat counts the mouse's movement in two 8-bit counters (vertical high, horizontal low); the buttons read low when held */
static void handleirq(unsigned int irq)
{

    unsigned short current = custom[REG_JOY0DAT / 2];
    char relx = (char)((current & 0xFF) - (position & 0xFF));
    char rely = (char)((current >> 8) - (position >> 8));
    unsigned int potgor = custom[REG_POTGOR / 2];

    position = current;

    if (relx || rely)
        mouse_notifymove(&mouseinterface, relx, rely);

    updatebutton(1, !(cia[REG_PRA] & REG_PRA_LEFT));
    updatebutton(2, !(potgor & REG_POTGOR_RIGHT));
    updatebutton(3, !(potgor & REG_POTGOR_MIDDLE));

}

static void driver_init(unsigned int id)
{

    mouse_initinterface(&mouseinterface, id);

}

static unsigned int driver_match(unsigned int id)
{

    return id == AMIGA_MOUSE;

}

static void driver_reset(unsigned int id)
{

}

/* polled once a frame, on the vertical blank */
static void driver_attach(unsigned int id)
{

    custom[REG_POTGO / 2] = REG_POTGO_OUTPUT;
    position = custom[REG_JOY0DAT / 2];

    mouse_registerinterface(&mouseinterface);
    amiga_setroutine(AMIGA_IRQ_VERTB, handleirq);

}

static void driver_detach(unsigned int id)
{

    mouse_unregisterinterface(&mouseinterface);
    amiga_unsetroutine(AMIGA_IRQ_VERTB, handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "amiga-mouse", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, AMIGA_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, AMIGA_BUS);

}

