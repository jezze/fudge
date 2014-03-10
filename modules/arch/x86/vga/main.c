#include <module.h>
#include <base/base.h>
#include <base/terminal.h>
#include <base/video.h>
#include "driver.h"

static struct vga_driver driver;

void init()
{

    vga_init_driver(&driver);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);

}

