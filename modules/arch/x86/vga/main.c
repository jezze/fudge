#include <module.h>
#include <base/base.h>
#include <base/terminal.h>
#include <base/video.h>
#include "driver.h"

void init()
{

    vga_driver_init();

}

void destroy()
{

    vga_driver_destroy();

}

