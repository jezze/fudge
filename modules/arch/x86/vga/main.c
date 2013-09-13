#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <terminal/terminal.h>
#include <video/video.h>
#include "vga.h"

static struct vga_driver driver;

void init()
{

    vga_init_driver(&driver);
    base_register_driver(&driver.base);
    terminal_register_interface(&driver.iterminal);

}

void destroy()
{

    terminal_unregister_interface(&driver.iterminal);
    base_unregister_driver(&driver.base);

}

