#include <base/base.h>
#include <video/video.h>
#include "vga.h"

static struct vga_driver driver;

void init()
{

    vga_init_driver(&driver);
    video_register_interface(&driver.interface);
    base_register_driver(&driver.base);

}

void destroy()
{

    video_unregister_interface(&driver.interface);
    base_unregister_driver(&driver.base);

}

