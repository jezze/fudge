#include <module.h>
#include <base/base.h>
#include <base/terminal.h>
#include <base/video.h>
#include "driver.h"

void init()
{

    cirrus_driver_init();

}

void destroy()
{

    cirrus_driver_destroy();

}

