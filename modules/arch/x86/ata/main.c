#include <module.h>
#include <base/base.h>
#include <base/block.h>
#include "driver.h"

void init()
{

    ata_driver_init();

}

void destroy()
{

    ata_driver_destroy();

}

