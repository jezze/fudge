#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/keyboard.h>
#include "usb.h"

#define CLASS_HID                       0x03
#define SUBCLASS_BOOT                   0x01
#define PROTOCOL_KEYBOARD               0x01
#define REQUEST_SETIDLE                 0x0A
#define REQUEST_SETPROTOCOL             0x0B
#define REPORTSIZE                      8
#define IDLE                            25
#define REPEATDELAY                     5
#define EXTENDED                        0xE000

static struct base_driver driver;
static struct keyboard_interface keyboardinterface;
static unsigned char previous[REPORTSIZE];
static unsigned char repeatkey;
static unsigned int repeatcount;

static unsigned short scancodes[] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x001E, 0x0030, 0x002E, 0x0020,
    0x0012, 0x0021, 0x0022, 0x0023, 0x0017, 0x0024, 0x0025, 0x0026,
    0x0032, 0x0031, 0x0018, 0x0019, 0x0010, 0x0013, 0x001F, 0x0014,
    0x0016, 0x002F, 0x0011, 0x002D, 0x0015, 0x002C, 0x0002, 0x0003,
    0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B,
    0x001C, 0x0001, 0x000E, 0x000F, 0x0039, 0x000C, 0x000D, 0x001A,
    0x001B, 0x002B, 0x002B, 0x0027, 0x0028, 0x0029, 0x0033, 0x0034,
    0x0035, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, 0x0040,
    0x0041, 0x0042, 0x0043, 0x0044, 0x0057, 0x0058, 0xE037, 0x0046,
    0x0000, 0xE052, 0xE047, 0xE049, 0xE053, 0xE04F, 0xE051, 0xE04D,
    0xE04B, 0xE050, 0xE048, 0x0045, 0xE035, 0x0037, 0x004A, 0x004E,
    0xE01C, 0x004F, 0x0050, 0x0051, 0x004B, 0x004C, 0x004D, 0x0047,
    0x0048, 0x0049, 0x0052, 0x0053, 0x0056, 0xE05D
};

static unsigned short modifiers[] = {
    0x001D, 0x002A, 0x0038, 0xE05B, 0xE01D, 0x0036, 0xE038, 0xE05C
};

static void notify(unsigned short scancode, unsigned int pressed)
{

    if (!scancode)
        return;

    if (pressed)
    {

        if (scancode & EXTENDED)
            keyboard_notifypress(&keyboardinterface, 0xE0);

        keyboard_notifypress(&keyboardinterface, scancode & 0xFF);

    }

    else
    {

        if (scancode & EXTENDED)
            keyboard_notifyrelease(&keyboardinterface, 0xE0);

        keyboard_notifyrelease(&keyboardinterface, (scancode & 0xFF) | 0x80);

    }

}

static unsigned short getscancode(unsigned char usage)
{

    return (usage < sizeof (scancodes) / sizeof (unsigned short)) ? scancodes[usage] : 0;

}

static unsigned int haskey(unsigned char *report, unsigned char usage)
{

    unsigned int i;

    for (i = 2; i < REPORTSIZE; i++)
    {

        if (report[i] == usage)
            return 1;

    }

    return 0;

}

static void onreport(unsigned int id, void *data, unsigned int count)
{

    unsigned char *report = data;
    unsigned int i;

    if (count < REPORTSIZE)
        return;

    if (report[2] == 0x01)
        return;

    if (buffer_match(report, previous, REPORTSIZE))
    {

        if (repeatkey && ++repeatcount >= REPEATDELAY)
            notify(getscancode(repeatkey), 1);

        return;

    }

    for (i = 0; i < 8; i++)
    {

        unsigned int bit = 1 << i;

        if ((report[0] & bit) != (previous[0] & bit))
            notify(modifiers[i], report[0] & bit);

    }

    for (i = 2; i < REPORTSIZE; i++)
    {

        if (previous[i] && !haskey(report, previous[i]))
        {

            notify(getscancode(previous[i]), 0);

            if (previous[i] == repeatkey)
                repeatkey = 0;

        }

    }

    for (i = 2; i < REPORTSIZE; i++)
    {

        if (report[i] && !haskey(previous, report[i]))
        {

            notify(getscancode(report[i]), 1);

            repeatkey = report[i];
            repeatcount = 0;

        }

    }

    buffer_copy(previous, report, REPORTSIZE);

}

static void driver_init(unsigned int id)
{

    keyboard_initinterface(&keyboardinterface, id);

}

static unsigned int driver_match(unsigned int id)
{

    return usb_getclass(id) == CLASS_HID && usb_getsubclass(id) == SUBCLASS_BOOT && usb_getprotocol(id) == PROTOCOL_KEYBOARD;

}

static void driver_reset(unsigned int id)
{

    unsigned int interface = usb_getinterface(id);

    usb_control(id, USB_REQUEST_OUT | USB_REQUEST_CLASS | USB_REQUEST_INTERFACE, REQUEST_SETPROTOCOL, 0, interface, 0, 0);
    usb_control(id, USB_REQUEST_OUT | USB_REQUEST_CLASS | USB_REQUEST_INTERFACE, REQUEST_SETIDLE, IDLE << 8, interface, 0, 0);

}

static void driver_attach(unsigned int id)
{

    keyboard_registerinterface(&keyboardinterface);
    usb_listen(id, onreport);

}

static void driver_detach(unsigned int id)
{

    usb_unlisten(id);
    keyboard_unregisterinterface(&keyboardinterface);

}

void module_init(void)
{

    base_initdriver(&driver, "usbkeyboard", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, USB_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, USB_BUS);

}

