#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

static struct mbr_driver driver;

void init()
{

    mbr_driver_init(&driver);
    modules_register_driver(&driver.base);
    modules_attach(&driver.base);

//    driver.base.start(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

