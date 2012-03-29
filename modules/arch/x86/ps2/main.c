#include <kernel/modules.h>
#include <modules/ps2/ps2.h>

static struct kbd_device kbdDevice;
static struct kbd_driver kbdDriver;
static struct mouse_device mouseDevice;
static struct mouse_driver mouseDriver;

void init()
{

    kbd_device_init(&kbdDevice, 0x01);
    modules_register_device(&kbdDevice.base);

    mouse_device_init(&mouseDevice, 0x0C);
    modules_register_device(&mouseDevice.base);

    kbd_driver_init(&kbdDriver);
    modules_register_driver(&kbdDriver.base);

    mouse_driver_init(&mouseDriver);
    modules_register_driver(&mouseDriver.base);

}

void destroy()
{

    modules_unregister_driver(&kbdDriver.base);
    modules_unregister_driver(&mouseDriver.base);

    modules_unregister_device(&kbdDevice.base);
    modules_unregister_device(&mouseDevice.base);

}

