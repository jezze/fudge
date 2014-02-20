#include <module.h>
#include <kernel/rendezvous.h>
#include <base/base.h>
#include <base/mouse.h>
#include <arch/x86/pic/pic.h>
#include "ps2.h"
#include "mouse_driver.h"

static unsigned int read_stream(struct ps2_mouse_stream *stream, unsigned int count, void *buffer)
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

static unsigned int write_stream(struct ps2_mouse_stream *stream, unsigned int count, void *buffer)
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
    struct ps2_mouse_driver *driver = (struct ps2_mouse_driver *)device->driver;
    unsigned char data = ps2_bus_read_data_async(bus);

    switch (driver->cycle)
    {

        case 0:

            driver->status = data;
            driver->cycle++;

            break;

        case 1:

            driver->imouse.vx = data;
            driver->cycle++;

            break;

        case 2:

            driver->imouse.vy = data;
            driver->cycle = 0;

            break;


    }

    write_stream(&driver->stream, 1, &data);
    rendezvous_unsleep(&driver->rdata, 1);

}

static void attach(struct base_device *device)
{

    struct ps2_bus *bus = (struct ps2_bus *)device->bus;
    struct ps2_mouse_driver *driver = (struct ps2_mouse_driver *)device->driver;
    unsigned char status;

    base_mouse_register_interface(&driver->imouse, device);
    pic_set_routine(device, handle_irq);
    ps2_bus_write_command(bus, 0xA8);
    ps2_bus_write_command(bus, 0x20);

    status = ps2_bus_read_data(bus) | 2;

    ps2_bus_write_command(bus, 0x60);
    ps2_bus_write_data(bus, status);
    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xF6);
    ps2_bus_read_data(bus);
    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xF4);
    ps2_bus_read_data(bus);

}

static unsigned int check(struct base_device *device)
{

    if (device->type != PS2_DEVICE_TYPE)
        return 0;

    return device->irq == PS2_IRQ_MOUSE;

}

static unsigned int read_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct ps2_mouse_driver *driver = (struct ps2_mouse_driver *)device->driver;

    count = read_stream(&driver->stream, count, buffer);

    rendezvous_sleep(&driver->rdata, !count);

    return count;

}

void ps2_init_mouse_driver(struct ps2_mouse_driver *driver)
{

    memory_clear(driver, sizeof (struct ps2_mouse_driver));
    base_init_driver(&driver->base, "ps2mouse", check, attach);
    base_mouse_init_interface(&driver->imouse, read_data);

    driver->cycle = 2;

}

