#include <memory.h>
#include <vfs.h>
#include <modules/modules.h>
#include <net/net.h>
#include <arch/x86/rtl8139/rtl8139.h>

static struct rtl8139_driver driver;

void init()
{

    rtl8139_driver_init(&driver);
    modules_register_driver(&driver.base);
    net_register_interface(&driver.interface, &driver.base);

}

void destroy()
{

    net_unregister_interface(&driver.interface);
    modules_unregister_driver(&driver.base);

}

