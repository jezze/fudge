#include <module.h>
#include <kernel/rendezvous.h>
#include <base/base.h>
#include <base/mouse.h>
#include <arch/x86/pic/pic.h>
#include "ps2.h"
#include "mouse_driver.h"

struct ps2_mouse_stream
{

    char buffer[512];
    unsigned int head;
    unsigned int tail;

};

static struct base_driver driver;
static struct base_mouse_interface imouse;
static struct ps2_mouse_stream stream;
static unsigned char cycle;
static char status;

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
    unsigned char data = ps2_bus_read_data_async(bus);

    switch (cycle)
    {

    case 0:
        status = data;
        cycle++;

        break;

    case 1:
        imouse.vx = data;
        cycle++;

        break;

    case 2:
        imouse.vy = data;
        cycle = 0;

        break;

    }

    write_stream(&stream, 1, &data);
    rendezvous_unsleep(&imouse.rdata, 1);

}

static void reset(struct ps2_bus *bus)
{

    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xFF);
    ps2_bus_read_data(bus);

}

static void set_defaults(struct ps2_bus *bus)
{

    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xF6);
    ps2_bus_read_data(bus);

}

static void identify(struct ps2_bus *bus)
{

    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xF2);
    ps2_bus_read_data(bus);

}

static void enable_scanning(struct ps2_bus *bus)
{

    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xF4);
    ps2_bus_read_data(bus);

}

static void disable_scanning(struct ps2_bus *bus)
{

    ps2_bus_write_command(bus, 0xD4);
    ps2_bus_write_data(bus, 0xF5);
    ps2_bus_read_data(bus);

}

static unsigned int read_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    count = read_stream(&stream, count, buffer);

    rendezvous_sleep(&imouse.rdata, !count);

    return count;

}

static void attach(struct base_device *device)
{

    struct ps2_bus *bus = (struct ps2_bus *)device->bus;

    base_mouse_init_interface(&imouse, read_data);
    base_mouse_register_interface(&imouse, device);
    pic_set_routine(device, handle_irq);
    ps2_device_enable(device);
    ps2_device_enable_interrupt(device);
    disable_scanning(bus);
    reset(bus);
    set_defaults(bus);
    identify(bus);
    enable_scanning(bus);

}

static unsigned int check(struct base_device *device)
{

    if (device->type != PS2_DEVICE_TYPE)
        return 0;

    return device->irq == PS2_IRQ_MOUSE;

}

void ps2_mouse_driver_init()
{

    base_init_driver(&driver, "ps2mouse", check, attach);
    base_register_driver(&driver);

}

void ps2_mouse_driver_destroy()
{

    base_unregister_driver(&driver);

}

