#include <modules.h>
#include <msr/msr.h>

static struct msr_driver driver;

void init()
{

    msr_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

