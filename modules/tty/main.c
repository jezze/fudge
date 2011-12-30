#include <kernel/modules.h>
#include <modules/ps2/ps2.h>
#include <modules/vga/vga.h>
#include <modules/tty/tty.h>

static struct tty_driver driver;

struct tty_driver *tty_get()
{

    return &driver;

}

void init()
{

    tty_driver_init(&driver, "home");
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

