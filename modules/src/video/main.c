#include <vfs.h>
#include <modules/modules.h>
#include <video/video.h>

static struct video_filesystem filesystem;

void video_register_interface(struct video_interface *interface, struct modules_driver *driver, unsigned int (*read_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_bpp)(struct video_interface *self), void (*write_bpp)(struct video_interface *self, unsigned int bpp))
{

    interface->driver = driver;
    interface->read_data = read_data;
    interface->write_data = write_data;
    interface->read_bpp = read_bpp;
    interface->write_bpp = write_bpp;

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

