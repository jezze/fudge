#include <system/system.h>
#include "video.h"
#include "group.h"

static struct video_group group;

void video_register_interface(struct video_interface *interface)
{

}

void video_register_protocol(struct video_protocol *protocol)
{

}

void video_unregister_interface(struct video_interface *interface)
{

}

void video_unregister_protocol(struct video_protocol *protocol)
{

}

void init()
{

    video_init_group(&group);
    system_register_group(&group.base);

}

void destroy()
{

    system_unregister_group(&group.base);

}

