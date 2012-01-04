#include <kernel/modules.h>
#include <modules/ps2/ps2.h>
#include <modules/vga/vga.h>
#include <modules/tty/tty.h>

static struct tty_driver driver;

void init()
{

    tty_driver_init(&driver, "home");
    modules_register_driver(&driver.base);

    tty_view_init(&driver.base.module);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

