#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "video.h"

static struct system_node root;

static unsigned int onvideoconf(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct event_videoconf *videoconf = data;

    interface->setmode(source, videoconf->width, videoconf->height, videoconf->bpp);

    return 1;

}

static unsigned int place(unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct video_interface *interface = (struct video_interface *)id;

    switch (event)
    {

    case EVENT_VIDEOCONF:
        return onvideoconf(interface, source, count, data);

    case EVENT_LINK:
        return kernel_link(interface->ichannel, source, interface->ichannel);

    case EVENT_UNLINK:
        return kernel_unlink(interface->ichannel, source);

    }

    return 0;

}

void video_notifymode(struct video_interface *interface, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct event_videoinfo videoinfo;

    videoinfo.framebuffer = framebuffer;
    videoinfo.width = w;
    videoinfo.height = h;
    videoinfo.bpp = bpp;

    kernel_notify(interface->ichannel, EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo);

}

void video_registerinterface(struct video_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->colormap);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);
    kernel_announce(interface->ichannel, (unsigned int)interface, place);

}

void video_unregisterinterface(struct video_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->colormap);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void video_initinterface(struct video_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*setmode)(unsigned int source, unsigned int width, unsigned int height, unsigned int bpp))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&interface->colormap, SYSTEM_NODETYPE_NORMAL, "colormap");
    system_initnode(&interface->event, SYSTEM_NODETYPE_NORMAL, "event");

    interface->id = id;
    interface->ichannel = ichannel;
    interface->width = 0;
    interface->height = 0;
    interface->bpp = 0;
    interface->setmode = setmode;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "video");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

