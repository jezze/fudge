#include <memory.h>
#include <string.h>
#include <modules.h>
#include <tty/tty.h>

static struct tty_driver driver;
static struct modules_filesystem filesystem;

struct modules_filesystem *get_filesystem()
{

    return &filesystem;

}

void init()
{

    tty_driver_init(&driver, "/");
    modules_register_driver(&driver.base);

    tty_filesystem_init(&filesystem, &driver.base);
    modules_register_filesystem(&filesystem);

}

void destroy()
{

    modules_unregister_filesystem(&filesystem);
    modules_unregister_driver(&driver.base);

}

