#include <vfs.h>
#include <base/base.h>
#include <net/net.h>
#include "rtl8139.h"

static struct rtl8139_driver driver;

void init()
{

    rtl8139_init_driver(&driver);
    base_register_driver(&driver.base);
    net_register_interface(&driver.interface);

}

void destroy()
{

    net_unregister_interface(&driver.interface);
    base_unregister_driver(&driver.base);

}

