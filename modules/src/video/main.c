#include <vfs.h>
#include <modules/modules.h>
#include <video/video.h>

static struct video_filesystem filesystem;

void video_register_interface(struct video_interface *interface, struct modules_driver *driver, unsigned int (*read)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    filesystem.register_interface(&filesystem, interface, driver, read, write);

}

void video_register_protocol(struct video_protocol *protocol, char *name)
{

    filesystem.register_protocol(&filesystem, protocol, name);

}

void video_unregister_interface(struct video_interface *interface)
{

    filesystem.unregister_interface(&filesystem, interface);

}

void video_unregister_protocol(struct video_protocol *protocol)
{

    filesystem.unregister_protocol(&filesystem, protocol);

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

