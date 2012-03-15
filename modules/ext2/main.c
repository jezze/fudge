#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver driver;
static struct ext2_filesystem filesystem;

void init()
{

    ext2_driver_init(&driver);
    modules_register_driver(&driver.base);

    ext2_filesystem_init(&filesystem, &driver);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

