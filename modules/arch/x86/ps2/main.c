#include <lib/memory.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>
#include <modules/ps2/ps2.h>

static struct kbd_device kbdDevice;
static struct kbd_driver kbdDriver;
static struct mouse_device mouseDevice;
static struct mouse_driver mouseDriver;

static struct nodefs_node mx;
static struct nodefs_node my;

static unsigned int mx_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, &mouseDriver.byte[1], 1);

    return 1;

}

static unsigned int my_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, &mouseDriver.byte[2], 1);

    return 1;

}

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

    struct nodefs_driver *nodefs = (struct nodefs_driver *)modules_get_driver(NODEFS_DRIVER_TYPE);

    if (!nodefs)
        return;

    nodefs->register_node(nodefs, &mx, "ps2/mx", &mouseDriver.base.module, mx_read, 0);
    nodefs->register_node(nodefs, &my, "ps2/my", &mouseDriver.base.module, my_read, 0);

}

void destroy()
{

    modules_unregister_driver(&kbdDriver.base);
    modules_unregister_driver(&mouseDriver.base);

    modules_unregister_device(&kbdDevice.base);
    modules_unregister_device(&mouseDevice.base);

}

