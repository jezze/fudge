#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver driver;

void ext2_driver_init(struct ext2_driver *driver)
{

    modules_driver_init(&driver->base, EXT2_DRIVER_TYPE);

}

void init()
{

    ext2_driver_init(&driver);

    modules_register_driver(&driver.base);

}

void destroy()
{

}

