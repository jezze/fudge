#include <module.h>
#include <kernel/resource.h>
#include <kernel/rendezvous.h>
#include <base/base.h>
#include <base/keyboard.h>
#include <arch/x86/pic/pic.h>
#include "ps2.h"
#include "keyboard_driver.h"

struct ps2_keyboard_stream
{

    char buffer[512];
    unsigned int head;
    unsigned int tail;

};

static struct base_driver driver;
static struct base_keyboard_interface ikeyboard;
static struct ps2_keyboard_stream stream;

static unsigned int read_stream(struct ps2_keyboard_stream *stream, unsigned int count, void *buffer)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int tail = (stream->tail + 1) % 512;

        if (stream->head == stream->tail)
            break;

        b[i] = stream->buffer[stream->tail];
        stream->tail = tail;

    }

    return i;

}

static unsigned int write_stream(struct ps2_keyboard_stream *stream, unsigned int count, void *buffer)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (stream->head + 1) % 512;

        if (head == stream->tail)
            break;

        stream->buffer[stream->head] = b[i];
        stream->head = head;

    }

    return i;

}

static unsigned int read_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    count = read_stream(&stream, count, buffer);

    rendezvous_sleep(&ikeyboard.rdata, !count);

    return count;

}

static unsigned int write_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return write_stream(&stream, count, buffer);

}

static void handle_irq(struct base_device *device)
{

    struct ps2_bus *bus = (struct ps2_bus *)device->bus;
    unsigned char scancode = ps2_bus_read_data_async(bus);

    if (ikeyboard.escaped)
        ikeyboard.escaped = 0;

    if (scancode == 0xE0)
        ikeyboard.escaped = 1;

    if (scancode & 0x80)
    {

        scancode &= ~0x80;

        if (scancode == 0x1D)
            ikeyboard.ctrl = 0;

        if (scancode == 0x2A)
            ikeyboard.shift = 0;

        if (scancode == 0x38)
            ikeyboard.alt = 0;

    }

    else
    {

        if (scancode == 0x1D)
            ikeyboard.ctrl = 1;

        if (scancode == 0x2A)
            ikeyboard.shift = 1;

        if (scancode == 0x38)
            ikeyboard.alt = 1;

        if (ikeyboard.ctrl)
            scancode = 0;

        if (ikeyboard.alt)
            scancode = 0;

        if (ikeyboard.shift)
            scancode += 128;

        write_stream(&stream, ikeyboard.keymap[scancode].length, ikeyboard.keymap[scancode].value);

    }

    rendezvous_unsleep(&ikeyboard.rdata, stream.tail < stream.head);

}

static void attach(struct base_device *device)
{

    base_keyboard_init_interface(&ikeyboard, read_data, write_data);
    base_keyboard_register_interface(&ikeyboard, device);
    pic_set_routine(device, handle_irq);
    ps2_device_enable(device);
    ps2_device_enable_interrupt(device);

}

static unsigned int check(struct base_device *device)
{

    if (device->type != PS2_DEVICE_TYPE)
        return 0;

    return device->irq == PS2_IRQ_KEYBOARD;

}

void ps2_keyboard_driver_init()
{

    base_init_driver(&driver, "ps2keyboard", check, attach);
    base_register_driver(&driver);

}

void ps2_keyboard_driver_destroy()
{

    base_unregister_driver(&driver);

}

