#include <kernel/modules.h>
#include <modules/smp/smp.h>

struct smp_driver driver;

void init()
{

    smp_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

