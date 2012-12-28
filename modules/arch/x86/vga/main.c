#include <base/base.h>
#include <nodefs/nodefs.h>
#include <video/video.h>
#include "vga.h"

static struct vga_driver driver;

static struct nodefs_node buffer;

static unsigned int buffer_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return driver.interface.read_data(&driver.interface, offset, count, buffer);

}

static unsigned int buffer_write(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return driver.interface.write_data(&driver.interface, offset, count, buffer);

}

void init()
{

    vga_init_driver(&driver);
    video_register_interface(&driver.interface);
    base_register_driver(&driver.base);

    nodefs_register_node(&buffer, "vga_buffer", &driver.base.module, buffer_read, buffer_write);

}

void destroy()
{

    nodefs_unregister_node(&buffer);

    video_unregister_interface(&driver.interface);
    base_unregister_driver(&driver.base);

}

