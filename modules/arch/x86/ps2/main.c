#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <base/base.h>
#include <system/system.h>
#include <kbd/kbd.h>
#include <mouse/mouse.h>
#include "ps2.h"

static struct ps2_bus bus;
static struct ps2_kbd_driver kbd;
static struct ps2_mouse_driver mouse;
static struct system_stream buffer;
static struct system_stream reset;

static unsigned int buffer_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return kbd.interface.read(&kbd.interface, offset, count, buffer);

}

static unsigned int buffer_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return kbd.interface.write(&kbd.interface, offset, count, buffer);

}

static unsigned int reset_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int reset_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    ps2_bus_reset(&bus);

    return 0;

}

void init()
{

    unsigned int i;

    ps2_init_bus(&bus);
    base_register_bus(&bus.base);

    for (i = 0; i < bus.devices.count; i++)
        base_register_device(&bus.devices.item[i].base);

    ps2_init_kbd_driver(&kbd);
    base_register_driver(&kbd.base);
    kbd_register_interface(&kbd.interface);
    ps2_init_mouse_driver(&mouse);
    base_register_driver(&mouse.base);
    mouse_register_interface(&mouse.interface);
    system_init_stream(&buffer, "ps2_buffer", buffer_read, buffer_write);
    system_register_node(&buffer.node);
    system_init_stream(&reset, "ps2_reset", reset_read, reset_write);
    system_register_node(&reset.node);

}

void destroy()
{

    unsigned int i;

    system_unregister_node(&buffer.node);
    system_unregister_node(&reset.node);
    kbd_unregister_interface(&kbd.interface);
    base_unregister_driver(&kbd.base);
    mouse_unregister_interface(&mouse.interface);
    base_unregister_driver(&mouse.base);

    for (i = 0; i < bus.devices.count; i++)
        base_unregister_device(&bus.devices.item[i].base);

    base_unregister_bus(&bus.base);

}

