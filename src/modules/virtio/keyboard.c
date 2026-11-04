#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/keyboard.h>
#include "virtio.h"

#define CONFIG_SELECT                   0x00
#define CONFIG_SUBSELECT                0x01
#define CONFIG_SIZE                     0x02
#define CONFIG_EVBITS                   0x11
#define EVENTS                          64
#define EV_KEY                          0x01
#define EV_REL                          0x02
#define KEYS                            128
#define EXTENDED                        0x100

struct input_event
{

    unsigned short type;
    unsigned short code;
    unsigned int value;

};

static struct base_driver driver;
static struct keyboard_interface keyboardinterface;
static struct virtio_device device;
static struct virtio_queue queue;
static unsigned char memory[VIRTIO_QUEUEMEMORY];
static struct input_event events[EVENTS];
static unsigned int msi;

/* Linux key codes up to F12 are scancode set 1 already; past them only the 0xE0 keys are kept */
static unsigned short scancodes[KEYS] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x00, 0x00, 0x56, 0x57, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    EXTENDED | 0x1C, EXTENDED | 0x1D, EXTENDED | 0x35, EXTENDED | 0x37, EXTENDED | 0x38, 0x00, EXTENDED | 0x47, EXTENDED | 0x48,
    EXTENDED | 0x49, EXTENDED | 0x4B, EXTENDED | 0x4D, EXTENDED | 0x4F, EXTENDED | 0x50, EXTENDED | 0x51, EXTENDED | 0x52, EXTENDED | 0x53,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, EXTENDED | 0x5B, EXTENDED | 0x5C, EXTENDED | 0x5D
};

static void handleevent(struct input_event *event)
{

    if (event->type == EV_KEY && event->code < KEYS)
    {

        unsigned short scancode;

        scancode = scancodes[event->code];

        if (!scancode)
            return;

        if (event->value)
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

static void driver_init(unsigned int id)
{

    keyboard_initinterface(&keyboardinterface);

}

/* keyboards and mice are both virtio input devices: a mouse is the one with relative axes */
static unsigned int driver_match(unsigned int type, unsigned int id)
{

    struct virtio_device probe;

    if (!virtio_match(type, id, VIRTIO_TYPE_INPUT) || !virtio_setup(&probe, id))
        return 0;

    return !hasevents(&probe, EV_REL);

}

/* the event queue gets one device-writable buffer per event */
static void driver_reset(unsigned int id)
{

    if (!virtio_setup(&device, id))
        return;

    virtio_negotiate(&device, 0);

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

    keyboard_registerinterface(&keyboardinterface);

    msi = virtio_setinterrupt(&device, 1, handleirq);

    virtio_setstatus(&device, VIRTIO_STATUS_DRIVEROK);
    virtio_notify(&queue);

}

static void driver_detach(unsigned int id)
{

    keyboard_unregisterinterface(&keyboardinterface);
    virtio_reset(&device);
    virtio_unsetinterrupt(&device, msi, handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "virtio-keyboard", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

