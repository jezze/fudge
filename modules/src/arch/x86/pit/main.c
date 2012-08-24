#include <memory.h>
#include <modules/modules.h>
#include <arch/x86/pit/pit.h>

static struct pit_device device;
static struct pit_driver driver;

void init()
{

    pit_device_init(&device, PIT_IRQ);
    modules_register_device(&device.base);

    pit_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);
    modules_unregister_device(&device.base);

}

