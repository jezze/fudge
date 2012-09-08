#include <memory.h>
#include <vfs.h>
#include <base/base.h>
#include <video/video.h>
#include <arch/x86/bga/bga.h>

static struct bga_driver driver;

void init()
{

    bga_driver_init(&driver);
    base_register_driver(&driver.base);
    video_register_interface(&driver.interface, &driver.base);

}

void destroy()
{

    video_unregister_interface(&driver.interface);
    base_unregister_driver(&driver.base);

}

