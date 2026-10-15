#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/mouse.h>
#include "usb.h"

#define CLASS_HID                       0x03
#define SUBCLASS_BOOT                   0x01
#define PROTOCOL_MOUSE                  0x02
#define REQUEST_SETIDLE                 0x0A
#define REQUEST_SETPROTOCOL             0x0B

static struct base_driver driver;
static struct mouse_interface mouseinterface;
static unsigned char oldbuttons;

static void onreport(unsigned int id, void *data, unsigned int count)
{

    unsigned char *report = data;
    unsigned char buttons;
    char relx;
    char rely;
    unsigned int i;

    if (count < 3)
        return;

    buttons = report[0];
    relx = report[1];
    rely = report[2];

    for (i = 0; i < 3; i++)
    {

        unsigned int bit = 1 << i;

        if (!(oldbuttons & bit) && (buttons & bit))
            mouse_notifypress(&mouseinterface, i + 1);

        if ((oldbuttons & bit) && !(buttons & bit))
            mouse_notifyrelease(&mouseinterface, i + 1);

    }

    oldbuttons = buttons;

    /* y grows downwards, as the mouse interface wants it */
    if (relx || rely)
        mouse_notifymove(&mouseinterface, relx, rely);

    /* the wheel is positive away from the user, the opposite of a ps/2 mouse */
    if (count > 3 && report[3])
        mouse_notifyscroll(&mouseinterface, -(char)report[3]);

}

static void driver_init(unsigned int id)
{

    mouse_initinterface(&mouseinterface, id);

}

static unsigned int driver_match(unsigned int id)
{

    return usb_getclass(id) == CLASS_HID && usb_getsubclass(id) == SUBCLASS_BOOT && usb_getprotocol(id) == PROTOCOL_MOUSE;

}

static void driver_reset(unsigned int id)
{

    unsigned int interface = usb_getinterface(id);

    usb_control(id, USB_REQUEST_OUT | USB_REQUEST_CLASS | USB_REQUEST_INTERFACE, REQUEST_SETPROTOCOL, 0, interface, 0, 0);
    usb_control(id, USB_REQUEST_OUT | USB_REQUEST_CLASS | USB_REQUEST_INTERFACE, REQUEST_SETIDLE, 0, interface, 0, 0);

}

static void driver_attach(unsigned int id)
{

    mouse_registerinterface(&mouseinterface);
    usb_listen(id, onreport);

}

static void driver_detach(unsigned int id)
{

    usb_unlisten(id);
    mouse_unregisterinterface(&mouseinterface);

}

void module_init(void)
{

    base_initdriver(&driver, "usbmouse", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, USB_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, USB_BUS);

}

