#include <kernel/modules.h>
#include <modules/fpu/fpu.h>

static struct fpu_driver driver;

void init()
{

    fpu_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

