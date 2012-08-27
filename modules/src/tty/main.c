#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <modules/modules.h>
#include <tty/tty.h>

static struct tty_driver driver;
static struct tty_filesystem filesystem;

struct vfs_interface *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    tty_driver_init(&driver, "/");
    modules_register_driver(&driver.base);

    tty_filesystem_init(&filesystem, &driver);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

