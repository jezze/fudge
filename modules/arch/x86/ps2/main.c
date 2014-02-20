#include <module.h>
#include <kernel/rendezvous.h>
#include <system/system.h>
#include <base/base.h>
#include <base/keyboard.h>
#include <base/mouse.h>
#include "ps2.h"
#include "keyboard_driver.h"
#include "mouse_driver.h"

enum ps2_register
{

    PS2_REGISTER_DATA                   = 0x0060,
    PS2_REGISTER_CONTROL                = 0x0064

};

static struct ps2_bus bus;
static struct ps2_keyboard_driver keyboard;
static struct ps2_mouse_driver mouse;
static struct system_stream reset;

static unsigned int reset_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    ps2_bus_reset(&bus);

    return 0;

}

void init()
{

    unsigned int i;

    ps2_init_bus(&bus, PS2_REGISTER_CONTROL, PS2_REGISTER_DATA);
    base_register_bus(&bus.base);

    for (i = 0; i < bus.devices.count; i++)
        base_register_device(&bus.devices.item[i].base);

    ps2_init_keyboard_driver(&keyboard);
    base_register_driver(&keyboard.base);
    ps2_init_mouse_driver(&mouse);
    base_register_driver(&mouse.base);
    system_init_stream(&reset, "reset");
    system_register_node(&reset.node);

    reset.node.write = reset_write;

}

void destroy()
{

    unsigned int i;

    system_unregister_node(&reset.node);
    base_unregister_driver(&keyboard.base);
    base_unregister_driver(&mouse.base);

    for (i = 0; i < bus.devices.count; i++)
        base_unregister_device(&bus.devices.item[i].base);

    base_unregister_bus(&bus.base);

}

