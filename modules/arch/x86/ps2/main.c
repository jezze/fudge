#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <system/system.h>
#include <base/base.h>
#include <kbd/kbd.h>
#include <mouse/mouse.h>
#include "ps2.h"

static struct ps2_bus bus;
static struct ps2_kbd_driver kbd;
static struct ps2_mouse_driver mouse;

static struct system_stream buffer;
static struct system_stream reset;

static unsigned int buffer_read(unsigned int offset, unsigned int count, void *buffer)
{

    return kbd.interface.read(&kbd.interface, offset, count, buffer);

}

static unsigned int buffer_write(unsigned int offset, unsigned int count, void *buffer)
{

    return kbd.interface.write(&kbd.interface, offset, count, buffer);

}

static unsigned int reset_read(unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int reset_write(unsigned int offset, unsigned int count, void *buffer)
{

    bus.reset();

    return 0;

}

void init()
{

    ps2_init_bus(&bus);
    base_register_bus(&bus.base);
    ps2_init_kbd_driver(&kbd);
    kbd_register_interface(&kbd.interface);
    base_register_driver(&kbd.base);
    ps2_init_mouse_driver(&mouse);
    mouse_register_interface(&mouse.interface);
    base_register_driver(&mouse.base);
    system_init_stream(&buffer, "ps2_buffer", buffer_read, buffer_write);
    system_register_node(&buffer.node);
    system_init_stream(&reset, "ps2_reset", reset_read, reset_write);
    system_register_node(&reset.node);

}

void destroy()
{

    system_unregister_node(&reset.node);
    system_unregister_node(&buffer.node);
    kbd_unregister_interface(&kbd.interface);
    base_unregister_driver(&kbd.base);
    mouse_unregister_interface(&mouse.interface);
    base_unregister_driver(&mouse.base);
    base_unregister_bus(&bus.base);

}

