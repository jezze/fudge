#include <modules.h>
#include <arch/x86/io/io.h>

static struct io_driver driver;

void init()
{

    io_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

