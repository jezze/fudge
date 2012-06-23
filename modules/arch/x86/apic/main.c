#include <kernel/modules.h>
#include <modules/apic/apic.h>

static struct apic_driver driver;

unsigned int apic_register_routine(unsigned int index, struct modules_device *device, void (*callback)(struct modules_device *device))
{

    return driver.register_routine(&driver, index, device, callback);

}

unsigned int apic_unregister_routine(unsigned int index, struct modules_device *device)
{

    return driver.unregister_routine(&driver, index, device);

}

void init()
{

    apic_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

