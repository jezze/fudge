#include <vfs.h>
#include <modules/modules.h>
#include <video/video.h>

static struct video_filesystem filesystem;

void video_register_interface(struct video_interface *interface, struct modules_driver *driver, unsigned int (*read_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_bpp)(struct video_interface *self), void (*write_bpp)(struct video_interface *self, unsigned int bpp), unsigned int (*read_xres)(struct video_interface *self), void (*write_xres)(struct video_interface *self, unsigned int xres), unsigned int (*read_yres)(struct video_interface *self), void (*write_yres)(struct video_interface *self, unsigned int yres))
{

    interface->driver = driver;
    interface->read_bpp = read_bpp;
    interface->read_data = read_data;
    interface->read_xres = read_xres;
    interface->read_yres = read_yres;
    interface->write_bpp = write_bpp;
    interface->write_data = write_data;
    interface->write_xres = write_xres;
    interface->write_yres = write_yres;

    filesystem.interfaces[filesystem.interfacesCount] = interface;
    filesystem.interfacesCount++;

}

void video_register_protocol(struct video_protocol *protocol, char *name)
{

    protocol->name = name;

    filesystem.protocols[filesystem.protocolsCount] = protocol;
    filesystem.protocolsCount++;

}

void video_unregister_interface(struct video_interface *interface)
{

}

void video_unregister_protocol(struct video_protocol *protocol)
{

}

struct vfs_interface *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    video_filesystem_init(&filesystem);

}

void destroy()
{

}

