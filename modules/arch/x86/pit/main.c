#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "pit.h"

static struct pit_device device;
static struct pit_driver driver;

void init()
{

    pit_init_device(&device, PIT_IRQ);
    pit_init_driver(&driver);
    base_register_device(&device.base);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);
    base_unregister_device(&device.base);

}

