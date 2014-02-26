#include <module.h>
#include <kernel/rendezvous.h>
#include <base/base.h>
#include <base/keyboard.h>
#include <arch/x86/pic/pic.h>
#include "ps2.h"
#include "keyboard_driver.h"

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

static void handle_irq(struct base_device *device)
{

    struct ps2_bus *bus = (struct ps2_bus *)device->bus;
    struct ps2_keyboard_driver *driver = (struct ps2_keyboard_driver *)device->driver;
    unsigned char scancode = ps2_bus_read_data_async(bus);

    if (driver->escaped)
        driver->escaped = 0;

    if (scancode == 0xE0)
        driver->escaped = 1;

    if (scancode & 0x80)
    {

        scancode &= ~0x80;

        if (scancode == 0x1D)
            driver->ctrl = 0;

        if (scancode == 0x2A)
            driver->shift = 0;

        if (scancode == 0x38)
            driver->alt = 0;

    }

    else
    {

        if (scancode == 0x1D)
            driver->ctrl = 1;

        if (scancode == 0x2A)
            driver->shift = 1;

        if (scancode == 0x38)
            driver->alt = 1;

        if (driver->ctrl)
            scancode = 0;

        if (driver->alt)
            scancode = 0;

        if (driver->shift)
            scancode += 128;

        write_stream(&driver->stream, driver->ikeyboard.keymap[scancode].length, driver->ikeyboard.keymap[scancode].value);
        rendezvous_unsleep(&driver->rdata, driver->ikeyboard.keymap[scancode].length);

    }

}

static void attach(struct base_device *device)
{

    struct ps2_keyboard_driver *driver = (struct ps2_keyboard_driver *)device->driver;

    base_keyboard_register_interface(&driver->ikeyboard, device);
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

static unsigned int read_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct ps2_keyboard_driver *driver = (struct ps2_keyboard_driver *)device->driver;

    count = read_stream(&driver->stream, count, buffer);

    rendezvous_sleep(&driver->rdata, !count);

    return count;

}

static unsigned int write_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct ps2_keyboard_driver *driver = (struct ps2_keyboard_driver *)device->driver;

    return write_stream(&driver->stream, count, buffer);

}

void ps2_init_keyboard_driver(struct ps2_keyboard_driver *driver)
{

    memory_clear(driver, sizeof (struct ps2_keyboard_driver));
    base_init_driver(&driver->base, "ps2keyboard", check, attach);
    base_keyboard_init_interface(&driver->ikeyboard, read_data, write_data);

}

