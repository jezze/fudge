#include <kernel/modules.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

static struct mbr_driver driver;

void init()
{

    mbr_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

