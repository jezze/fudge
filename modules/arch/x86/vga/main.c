#include <fudge/module.h>
#include <base/base.h>
#include <terminal/terminal.h>
#include <video/video.h>
#include "vga.h"

static struct vga_driver driver;

void init()
{

    vga_init_driver(&driver);
    base_register_driver(&driver.base);
    video_register_interface(&driver.video);
    terminal_register_interface(&driver.terminal);

}

void destroy()
{

    terminal_unregister_interface(&driver.terminal);
    video_unregister_interface(&driver.video);
    base_unregister_driver(&driver.base);

}

