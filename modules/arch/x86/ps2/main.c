#include <lib/memory.h>
#include <kernel/modules.h>
#include <modules/nodefs/nodefs.h>
#include <modules/ps2/ps2.h>

static struct ps2_kbd_device kbdDevice;
static struct ps2_kbd_driver kbdDriver;
static struct ps2_mouse_device mouseDevice;
static struct ps2_mouse_driver mouseDriver;

static struct nodefs_node buffer;
static struct nodefs_node mcycle;
static struct nodefs_node mstatus;
static struct nodefs_node mx;
static struct nodefs_node my;

unsigned int ps2_getc(char *buffer)
{

    return kbdDriver.buffer.getc(&kbdDriver.buffer, buffer);

}

unsigned int ps2_putc(char *buffer)
{

    return kbdDriver.buffer.putc(&kbdDriver.buffer, buffer);

}

static unsigned int buffer_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return kbdDriver.buffer.getc(&kbdDriver.buffer, buffer);

}

static unsigned int buffer_write(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return kbdDriver.buffer.putc(&kbdDriver.buffer, buffer);

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

    ps2_kbd_device_init(&kbdDevice, PS2_IRQ_KBD);
    modules_register_device(&kbdDevice.base);

    ps2_mouse_device_init(&mouseDevice, PS2_IRQ_MOUSE);
    modules_register_device(&mouseDevice.base);

    ps2_kbd_driver_init(&kbdDriver);
    modules_register_driver(&kbdDriver.base);

    ps2_mouse_driver_init(&mouseDriver);
    modules_register_driver(&mouseDriver.base);

    nodefs_register_node(&buffer, "ps2/buffer", &kbdDriver.base.base, buffer_read, buffer_write);
    nodefs_register_node(&mcycle, "ps2/mcycle", &mouseDriver.base.base, mcycle_read, 0);
    nodefs_register_node(&mstatus, "ps2/mstatus", &mouseDriver.base.base, mstatus_read, 0);
    nodefs_register_node(&mx, "ps2/mx", &mouseDriver.base.base, mx_read, 0);
    nodefs_register_node(&my, "ps2/my", &mouseDriver.base.base, my_read, 0);

}

void destroy()
{

    modules_unregister_driver(&kbdDriver.base);
    modules_unregister_driver(&mouseDriver.base);

    modules_unregister_device(&kbdDevice.base);
    modules_unregister_device(&mouseDevice.base);

}

