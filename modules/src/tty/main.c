#include <vfs.h>
#include <base/base.h>
#include <tty/tty.h>

static struct tty_driver driver;
static struct tty_filesystem filesystem;

struct vfs_interface *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    tty_init_driver(&driver, "/", 1);
    base_register_driver(&driver.base);

    tty_init_filesystem(&filesystem, &driver);

}

void destroy()
{

    base_unregister_driver(&driver.base);

}

