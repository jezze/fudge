#include <memory.h>
#include <vfs.h>
#include <modules/modules.h>
#include <video/video.h>
#include <arch/x86/bga/bga.h>

static struct bga_driver driver;

static void enable(struct video_interface *self)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;

    driver->set_mode(driver);

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
    video_register_interface(&driver.interface, &driver.base, enable, read_data, write_data, read_bpp, write_bpp, read_xres, write_xres, read_yres, write_yres);

}

void destroy()
{

    video_unregister_interface(&driver.interface);
    modules_unregister_driver(&driver.base);

}

