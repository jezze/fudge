#include <fudge/module.h>
#include <base/base.h>
#include <video/video.h>
#include "bga.h"

static struct bga_driver driver;

void init()
{

    bga_init_driver(&driver);
    base_register_driver(&driver.base);
    video_register_interface(&driver.ivideo);

}

void destroy()
{

    video_unregister_interface(&driver.ivideo);
    base_unregister_driver(&driver.base);

}

