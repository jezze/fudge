#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <block/block.h>
#include "ata.h"

static struct ata_driver driver;

void init()
{

    ata_init_driver(&driver);
    base_register_driver(&driver.base);
    block_register_interface(&driver.iblock);

}

void destroy()
{

    block_unregister_interface(&driver.iblock);
    base_unregister_driver(&driver.base);

}

