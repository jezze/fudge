#include <kernel/modules.h>
#include <modules/rtl8139/rtl8139.h>

static struct rtl8139_driver driver;

void init()
{

    rtl8139_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

