#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>
#include <modules/ps2/ps2.h>
#include <modules/tty/tty.h>

static struct tty_device device;

struct tty_device *tty_get()
{

    return &device;

}

void init()
{

    tty_device_init(&device, "home");

    modules_register_device(&device.base);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

