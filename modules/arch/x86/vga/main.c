#include <system/system.h>
#include <base/base.h>
#include <video/video.h>
#include "vga.h"

static struct vga_driver driver;

static struct system_stream buffer;

static unsigned int buffer_read(unsigned int offset, unsigned int count, void *buffer)
{

    return driver.interface.read(&driver.interface, offset, count, buffer);

}

static unsigned int buffer_write(unsigned int offset, unsigned int count, void *buffer)
{

    return driver.interface.write(&driver.interface, offset, count, buffer);

}

void init()
{

    vga_init_driver(&driver);
    base_register_driver(&driver.base);
    video_register_interface(&driver.interface);

    system_init_stream(&buffer, "vga_buffer", buffer_read, buffer_write);
    system_register_node(&buffer.node);

}

void destroy()
{

    system_unregister_node(&buffer.node);

    video_unregister_interface(&driver.interface);
    base_unregister_driver(&driver.base);

}

