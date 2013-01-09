#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <system/system.h>
#include <base/base.h>
#include <kbd/kbd.h>
#include <mouse/mouse.h>
#include "ps2.h"

static struct ps2_device kbdDevice;
static struct ps2_device mouseDevice;
static struct ps2_kbd_driver kbdDriver;
static struct ps2_mouse_driver mouseDriver;

static struct system_stream buffer;

static unsigned int buffer_read(unsigned int offset, unsigned int count, void *buffer)
{

    return kbdDriver.interface.read(&kbdDriver.interface, offset, count, buffer);

}

static unsigned int buffer_write(unsigned int offset, unsigned int count, void *buffer)
{

    return kbdDriver.interface.write(&kbdDriver.interface, offset, count, buffer);

}

void init()
{

    ps2_init_device(&kbdDevice, "ps2kbd", PS2_IRQ_KEYBOARD);
    ps2_init_device(&mouseDevice, "ps2mouse", PS2_IRQ_MOUSE);
    ps2_init_kbd_driver(&kbdDriver);
    ps2_init_mouse_driver(&mouseDriver);
    base_register_device(&kbdDevice.base);
    base_register_device(&mouseDevice.base);
    base_register_driver(&kbdDriver.base);
    base_register_driver(&mouseDriver.base);
    kbd_register_interface(&kbdDriver.interface);
    mouse_register_interface(&mouseDriver.interface);
    system_init_stream(&buffer, "ps2_buffer", buffer_read, buffer_write);
    system_register_node(&buffer.node);

}

void destroy()
{

    system_unregister_node(&buffer.node);
    kbd_unregister_interface(&kbdDriver.interface);
    mouse_unregister_interface(&mouseDriver.interface);
    base_unregister_driver(&kbdDriver.base);
    base_unregister_driver(&mouseDriver.base);
    base_unregister_device(&kbdDevice.base);
    base_unregister_device(&mouseDevice.base);

}

