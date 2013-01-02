#include <system/system.h>
#include <base/base.h>
#include <video/video.h>
#include "bga.h"

static struct bga_driver driver;

void init()
{

    bga_init_driver(&driver);
    base_register_driver(&driver.base);
    video_register_interface(&driver.interface);

}

void destroy()
{

    video_unregister_interface(&driver.interface);
    base_unregister_driver(&driver.base);

}

