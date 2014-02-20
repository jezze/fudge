#include <module.h>
#include <base/base.h>
#include <base/block.h>
#include "driver.h"

static struct atapi_driver driver;

void init()
{

    atapi_init_driver(&driver);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);

}

