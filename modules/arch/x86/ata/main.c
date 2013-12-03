#include <fudge/module.h>
#include <base/base.h>
#include <base/block.h>
#include "driver.h"

static struct ata_driver driver;

void init()
{

    ata_init_driver(&driver);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);

}

