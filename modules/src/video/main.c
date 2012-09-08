#include <vfs.h>
#include <modules/modules.h>
#include <video/video.h>

static struct video_filesystem filesystem;

void video_register_interface(struct video_interface *interface, struct modules_driver *driver)
{

    interface->driver = driver;

    filesystem.interfaces[filesystem.interfacesCount] = interface;
    filesystem.interfacesCount++;

}

void video_register_protocol(struct video_protocol *protocol)
{

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

