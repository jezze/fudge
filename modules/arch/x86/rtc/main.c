#include <module.h>
#include <base/base.h>
#include <base/clock.h>
#include "driver.h"

void init()
{

    rtc_driver_init();

}

void destroy()
{

    rtc_driver_destroy();

}

