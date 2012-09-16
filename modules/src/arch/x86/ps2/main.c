#include <memory.h>
#include <vfs.h>
#include <base/base.h>
#include <nodefs/nodefs.h>
#include <arch/x86/ps2/ps2.h>

static struct ps2_device kbdDevice;
static struct ps2_device mouseDevice;
static struct ps2_kbd_driver kbdDriver;
static struct ps2_mouse_driver mouseDriver;

static struct nodefs_node buffer;
static struct nodefs_node mcycle;
static struct nodefs_node mstatus;
static struct nodefs_node mx;
static struct nodefs_node my;

static unsigned int buffer_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return kbdDriver.buffer.read(&kbdDriver.buffer, count, buffer);

}

static unsigned int buffer_write(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return kbdDriver.buffer.write(&kbdDriver.buffer, count, buffer);

}

static unsigned int mcycle_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, &mouseDriver.cycle, 1);

    return 1;

}

static unsigned int mstatus_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, &mouseDriver.status, 1);

    return 1;

}

static unsigned int mx_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, &mouseDriver.x, 1);

    return 1;

}

static unsigned int my_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, &mouseDriver.y, 1);

    return 1;

}

void init()
{

    ps2_device_init(&kbdDevice, "ps2kbd", PS2_IRQ_KBD);
    ps2_device_init(&mouseDevice, "ps2mouse", PS2_IRQ_MOUSE);
    ps2_kbd_driver_init(&kbdDriver);
    ps2_mouse_driver_init(&mouseDriver);

    base_register_device(&kbdDevice.base);
    base_register_device(&mouseDevice.base);
    base_register_driver(&kbdDriver.base);
    base_register_driver(&mouseDriver.base);

    nodefs_register_node(&buffer, "ps2_buffer", &kbdDriver.base.module, buffer_read, buffer_write);
    nodefs_register_node(&mcycle, "ps2_mcycle", &mouseDriver.base.module, mcycle_read, 0);
    nodefs_register_node(&mstatus, "ps2_mstatus", &mouseDriver.base.module, mstatus_read, 0);
    nodefs_register_node(&mx, "ps2_mx", &mouseDriver.base.module, mx_read, 0);
    nodefs_register_node(&my, "ps2_my", &mouseDriver.base.module, my_read, 0);

}

void destroy()
{

    base_unregister_driver(&kbdDriver.base);
    base_unregister_driver(&mouseDriver.base);
    base_unregister_device(&kbdDevice.base);
    base_unregister_device(&mouseDevice.base);

}

