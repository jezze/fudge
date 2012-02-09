#include <kernel/modules.h>
#include <modules/vga/vga.h>

static struct vga_driver driver;

void init()
{

    vga_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

