#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/mbr/mbr.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver driver;
static struct ext2_filesystem filesystem;

void init()
{

    struct mbr_driver *mbrDriver = (struct mbr_driver *)modules_get_driver(MBR_DRIVER_TYPE);

    if (!mbrDriver)
        return;

    ext2_driver_init(&driver, mbrDriver);
    modules_register_driver(&driver.base);

    ext2_filesystem_init(&filesystem, &driver);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

