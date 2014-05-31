#include <module.h>
#include <base/base.h>
#include <base/block.h>
#include "driver.h"

void init()
{

    atapi_driver_init();

}

void destroy()
{

    atapi_driver_destroy();

}

