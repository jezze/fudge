#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/mouse.h>
#include "virtio.h"
#include "input.h"

#define REL_X                           0x00
#define REL_Y                           0x01
#define REL_WHEEL                       0x08
#define BTN_LEFT                        0x110
#define BTN_RIGHT                       0x111
#define BTN_MIDDLE                      0x112

static struct base_driver driver;
static struct mouse_interface mouseinterface;
static struct virtio_device device;
static struct virtio_queue queue;
static unsigned char memory[VIRTIO_QUEUEMEMORY];
static struct input_event events[INPUT_EVENTS];
static unsigned int msi;
static int relx;
static int rely;
static int relz;

static char clamp(int value)
{

    return (value > 127) ? 127 : (value < -127) ? -127 : value;

}

/* a report ends with EV_SYN: movement is sent in pieces that fit the interface's chars */
static void report(void)
{

    while (relx || rely)
    {

        char x = clamp(relx);
        char y = clamp(rely);

        mouse_notifymove(&mouseinterface, x, y);

        relx -= x;
        rely -= y;

    }

    if (relz)
        mouse_notifyscroll(&mouseinterface, clamp(-relz));

    relz = 0;

}

static void handleevent(struct input_event *event)
{

    switch (event->type)
    {

    case INPUT_EVSYN:
        report();

        break;

    case INPUT_EVREL:
        if (event->code == REL_X)
            relx += (int)event->value;
        else if (event->code == REL_Y)
            rely += (int)event->value;
        else if (event->code == REL_WHEEL)
            relz += (int)event->value;

        break;

    case INPUT_EVKEY:
        if (event->code >= BTN_LEFT && event->code <= BTN_MIDDLE)
        {

            unsigned int button = (event->code == BTN_LEFT) ? 1 : (event->code == BTN_RIGHT) ? 2 : 3;

            if (event->value)
                mouse_notifypress(&mouseinterface, button);
            else
                mouse_notifyrelease(&mouseinterface, button);

        }

        break;

    }

}

static void handleirq(unsigned int irq)
{

    input_poll(&device, &queue, msi, events, handleevent);

}

static void driver_init(unsigned int id)
{

    mouse_initinterface(&mouseinterface, id);

}

static unsigned int driver_match(unsigned int id)
{

    return input_match(id, INPUT_EVREL);

}

static void driver_reset(unsigned int id)
{

    input_setup(&device, &queue, id, memory, events);

}

static void driver_attach(unsigned int id)
{

    mouse_registerinterface(&mouseinterface);

    msi = virtio_setinterrupt(&device, 1, handleirq);

    virtio_setstatus(&device, VIRTIO_STATUS_DRIVEROK);
    virtio_notify(&queue);

}

static void driver_detach(unsigned int id)
{

    mouse_unregisterinterface(&mouseinterface);
    virtio_reset(&device);
    virtio_unsetinterrupt(&device, msi, handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "virtio-mouse", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    virtio_registerdriver(&driver);

}

void module_unregister(void)
{

    virtio_unregisterdriver(&driver);

}

