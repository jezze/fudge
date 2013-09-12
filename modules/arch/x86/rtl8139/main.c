#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <net/net.h>
#include "rtl8139.h"

static struct rtl8139_driver driver;

void init()
{

    rtl8139_init_driver(&driver);
    base_register_driver(&driver.base);
    net_register_interface(&driver.inet);

}

void destroy()
{

    net_unregister_interface(&driver.inet);
    base_unregister_driver(&driver.base);

}

