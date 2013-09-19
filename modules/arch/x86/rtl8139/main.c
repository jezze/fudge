#include <fudge/module.h>
#include <base/base.h>
#include <base/network.h>
#include "rtl8139.h"

static struct rtl8139_driver driver;

void init()
{

    rtl8139_init_driver(&driver);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);

}

