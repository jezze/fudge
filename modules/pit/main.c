#include <kernel/modules.h>
#include <modules/pit/pit.h>

static struct pit_device device;

void init()
{

    pit_device_init(&device);
    modules_register_device(&device.base);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

