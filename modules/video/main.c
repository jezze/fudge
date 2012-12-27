#include <kernel/vfs.h>
#include <base/base.h>
#include <system/system.h>
#include "video.h"

static struct system_group group;
static struct video_filesystem filesystem;

void video_register_interface(struct video_interface *interface)
{

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

    system_init_group(&group, "video");
    system_register_group(&group);
    video_init_filesystem(&filesystem);

}

void destroy()
{

    system_unregister_group(&group);

}

