#include <module.h>
#include <base/base.h>
#include <base/clock.h>
#include "driver.h"

static struct rtc_driver driver;

void init()
{

    rtc_init_driver(&driver);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);

}

