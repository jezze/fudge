#include <module.h>
#include <base/base.h>
#include <base/video.h>
#include "driver.h"

static struct bga_driver driver;

void init()
{

    bga_init_driver(&driver);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);

}

