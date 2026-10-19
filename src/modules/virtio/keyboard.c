#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/keyboard.h>
#include "virtio.h"
#include "input.h"

#define KEYS                            128
#define EXTENDED                        0x100

static struct base_driver driver;
static struct keyboard_interface keyboardinterface;
static struct virtio_device device;
static struct virtio_queue queue;
static unsigned char memory[VIRTIO_QUEUEMEMORY];
static struct input_event events[INPUT_EVENTS];
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

    unsigned short scancode;

    if (event->type != INPUT_EVKEY || event->code >= KEYS)
        return;

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

static void handleirq(unsigned int irq)
{

    input_poll(&device, &queue, msi, events, handleevent);

}

static void driver_init(unsigned int id)
{

    keyboard_initinterface(&keyboardinterface, id);

}

static unsigned int driver_match(unsigned int id)
{

    return input_match(id, INPUT_EVKEY);

}

static void driver_reset(unsigned int id)
{

    input_setup(&device, &queue, id, memory, events);

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

    virtio_registerdriver(&driver);

}

void module_unregister(void)
{

    virtio_unregisterdriver(&driver);

}

