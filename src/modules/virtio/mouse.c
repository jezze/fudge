#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/mouse.h>
#include "virtio.h"

#define CONFIG_SELECT                   0x00
#define CONFIG_SUBSELECT                0x01
#define CONFIG_SIZE                     0x02
#define CONFIG_EVBITS                   0x11
#define CONFIG_ABSINFO                  0x12
#define CONFIG_ABSMAX                   0x0C
#define EVENTS                          64
#define EV_SYN                          0x00
#define EV_KEY                          0x01
#define EV_REL                          0x02
#define EV_ABS                          0x03
#define ABS_X                           0x00
#define ABS_Y                           0x01
#define REL_X                           0x00
#define REL_Y                           0x01
#define REL_WHEEL                       0x08
#define BTN_LEFT                        0x110
#define BTN_RIGHT                       0x111
#define BTN_MIDDLE                      0x112

struct input_event
{

    unsigned short type;
    unsigned short code;
    unsigned int value;

};

static struct base_driver driver;
static struct mouse_interface mouseinterface;
static struct virtio_device device;
static struct virtio_queue queue;
static unsigned char memory[VIRTIO_QUEUEMEMORY];
static struct input_event events[EVENTS];
static unsigned int msi;
static int relx;
static int rely;
static int relz;
static unsigned int absx;
static unsigned int absy;
static unsigned int absmaxx;
static unsigned int absmaxy;
static unsigned int absmoved;

static char clamp(int value)
{

    return (value > 127) ? 127 : (value < -127) ? -127 : value;

}

static unsigned short scaleposition(unsigned int value, unsigned int max)
{

    return (max) ? ((value < max) ? value : max) * 65535 / max : 0;

}

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

    if (absmoved)
        mouse_notifyposition(&mouseinterface, scaleposition(absx, absmaxx), scaleposition(absy, absmaxy));

    absmoved = 0;

    if (relz)
        mouse_notifyscroll(&mouseinterface, clamp(-relz));

    relz = 0;

}

static void handleevent(struct input_event *event)
{

    switch (event->type)
    {

    case EV_SYN:
        report();

        break;

    case EV_REL:
        if (event->code == REL_X)
            relx += (int)event->value;
        else if (event->code == REL_Y)
            rely += (int)event->value;
        else if (event->code == REL_WHEEL)
            relz += (int)event->value;

        break;

    case EV_ABS:
        if (event->code == ABS_X)
            absx = event->value;
        else if (event->code == ABS_Y)
            absy = event->value;

        absmoved = 1;

        break;

    case EV_KEY:
        if (event->code >= BTN_LEFT && event->code <= BTN_MIDDLE)
        {

            unsigned int button = (event->code == BTN_LEFT) ? 1 : (event->code == BTN_RIGHT) ? 2 : 3;

            /* a tap is position, button, sync: the button goes where the pointer is meant to be */
            report();

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

    if (!msi)
        virtio_getisr(&device);

    while (virtio_pending(&queue))
    {

        unsigned int id = virtio_pop(&queue);

        if (id >= EVENTS)
            continue;

        events[id].type = virtio_le16(events[id].type);
        events[id].code = virtio_le16(events[id].code);
        events[id].value = virtio_le32(events[id].value);

        handleevent(&events[id]);
        virtio_push(&queue, id);

    }

    virtio_notify(&queue);

}

static unsigned int hasevents(struct virtio_device *device, unsigned int type)
{

    virtio_setconfig(device, CONFIG_SELECT, CONFIG_EVBITS);
    virtio_setconfig(device, CONFIG_SUBSELECT, type);

    return virtio_getconfig(device, CONFIG_SIZE) != 0;

}

static unsigned int getabsmax(struct virtio_device *device, unsigned int axis)
{

    virtio_setconfig(device, CONFIG_SELECT, CONFIG_ABSINFO);
    virtio_setconfig(device, CONFIG_SUBSELECT, axis);

    return virtio_getconfig(device, CONFIG_ABSMAX) | (virtio_getconfig(device, CONFIG_ABSMAX + 1) << 8) | (virtio_getconfig(device, CONFIG_ABSMAX + 2) << 16) | ((unsigned int)virtio_getconfig(device, CONFIG_ABSMAX + 3) << 24);

}

static void driver_init(unsigned int id)
{

    mouse_initinterface(&mouseinterface);

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    struct virtio_device probe;

    if (!virtio_match(type, id, VIRTIO_TYPE_INPUT) || !virtio_setup(&probe, id))
        return 0;

    return hasevents(&probe, EV_REL) || hasevents(&probe, EV_ABS);

}

static void driver_reset(unsigned int id)
{

    if (!virtio_setup(&device, id))
        return;

    virtio_negotiate(&device, 0);

    absmaxx = getabsmax(&device, ABS_X);
    absmaxy = getabsmax(&device, ABS_Y);

    if (virtio_setqueue(&queue, &device, 0, memory))
    {

        unsigned int i;

        for (i = 0; i < EVENTS && i < queue.size; i++)
        {

            virtio_setdesc(&queue, i, &events[i], sizeof (struct input_event), VIRTIO_DESC_WRITE, 0);
            virtio_push(&queue, i);

        }

    }

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

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

