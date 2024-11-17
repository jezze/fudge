#include <fudge.h>
#include <kernel.h>
#include "video.h"

static struct service service;

static struct resource *service_foreach(struct resource *current)
{

    return resource_foreachtype(current, RESOURCE_VIDEOINTERFACE);

}

static unsigned int service_getinode(struct resource *current, unsigned int index)
{

    struct video_interface *interface = current->data;

    return interface->inode;

}

static unsigned int onvideocmap(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    return interface->onvideocmap(source, count, data);

}

static unsigned int onvideoconf(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct event_videoconf *videoconf = data;

    return interface->onvideoconf(source, videoconf->width, videoconf->height, videoconf->bpp);

}

static unsigned int place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct video_interface *interface = kernel_getinterface(target);

    switch (event)
    {

    case EVENT_VIDEOCMAP:
        return onvideocmap(interface, source, count, data);

    case EVENT_VIDEOCONF:
        return onvideoconf(interface, source, count, data);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void video_notifymode(struct video_interface *interface, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct event_videoinfo videoinfo;

    videoinfo.framebuffer = framebuffer;
    videoinfo.width = w;
    videoinfo.height = h;
    videoinfo.bpp = bpp;

    kernel_notify(interface->inode, EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo);

}

void video_registerinterface(struct video_interface *interface)
{

    resource_register(&interface->resource);

}

void video_unregisterinterface(struct video_interface *interface)
{

    resource_unregister(&interface->resource);

}

void video_initinterface(struct video_interface *interface, unsigned int id, unsigned int (*onvideocmap)(unsigned int source, unsigned int count, void *data), unsigned int (*onvideoconf)(unsigned int source, unsigned int width, unsigned int height, unsigned int bpp))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);

    interface->id = id;
    interface->inode = kernel_link(0, 0, &interface->resource, place);
    interface->width = 0;
    interface->height = 0;
    interface->bpp = 0;
    interface->onvideocmap = onvideocmap;
    interface->onvideoconf = onvideoconf;

}

void module_init(void)
{

    service_init(&service, "video", service_foreach, service_getinode);
    service_register(&service);

}

