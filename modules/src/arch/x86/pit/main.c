#include <memory.h>
#include <base/base.h>
#include <arch/x86/pit/pit.h>

static struct pit_device device;
static struct pit_driver driver;

void init()
{

    pit_device_init(&device, PIT_IRQ);
    base_register_device(&device.base);

    pit_driver_init(&driver);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);
    base_unregister_device(&device.base);

}

