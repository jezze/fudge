#include <memory.h>
#include <vfs.h>
#include <modules/modules.h>
#include <nodefs/nodefs.h>
#include <video/video.h>
#include <arch/x86/bga/bga.h>

static struct bga_driver driver;
static struct nodefs_node enable;

static unsigned int enable_write(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    driver.set_mode(&driver);

    return 0;

}

static unsigned int read_bpp(struct video_interface *self)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;
    
    return driver->bpp;

}

static void write_bpp(struct video_interface *self, unsigned int bpp)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;

    driver->bpp = bpp;

}

static unsigned int read_data(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;

    memory_copy(buffer, (char *)driver->lfb + offset, count);

    return count;

}

static unsigned int write_data(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;

    memory_copy((char *)driver->lfb + offset, buffer, count);

    return count;

}

static unsigned int read_xres(struct video_interface *self)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;
    
    return driver->xres;

}

static void write_xres(struct video_interface *self, unsigned int xres)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;

    driver->xres = xres;

}

static unsigned int read_yres(struct video_interface *self)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;
    
    return driver->yres;

}

static void write_yres(struct video_interface *self, unsigned int yres)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;

    driver->yres = yres;

}

void init()
{

    bga_driver_init(&driver);
    modules_register_driver(&driver.base);
    video_register_interface(&driver.interface, &driver.base, read_data, write_data, read_bpp, write_bpp, read_xres, write_xres, read_yres, write_yres);

    nodefs_register_node(&enable, "bga_enable", &driver.base.base, 0, enable_write);

}

void destroy()
{

    video_unregister_interface(&driver.interface);
    modules_unregister_driver(&driver.base);

}

