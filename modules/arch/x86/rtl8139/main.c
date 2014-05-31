#include <module.h>
#include <base/base.h>
#include <base/network.h>
#include "driver.h"

void init()
{

    rtl8139_driver_init();

}

void destroy()
{

    rtl8139_driver_destroy();

}

