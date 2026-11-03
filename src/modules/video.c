#include <fudge.h>
#include <kernel.h>
#include "video.h"

static struct resource_operands operands;
static struct dirsrv dirsrv;
static struct service service;

static unsigned int oninfo(struct video_interface *interface, unsigned int source)
{

    if (interface->framebuffer)
    {

        struct event_videoinfo videoinfo;

        videoinfo.framebuffer = interface->framebuffer;
        videoinfo.width = interface->width;
        videoinfo.height = interface->height;
        videoinfo.bpp = interface->bpp;

        return kernel_place(interface->resource.inode, source, EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int onvideocmap(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->setcmap)
    {

        interface->setcmap(count / 3, data);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int onvideoconf(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (count < sizeof (struct event_videoconf))
        return MESSAGE_FAILED;

    if (interface->setmode)
    {

        struct event_videoconf *videoconf = data;

        interface->setmode(videoconf->width, videoconf->height, videoconf->bpp);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

/* only displays that show a copy of the framebuffer need to be told what changed */
static unsigned int onvideoflush(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->flush && count == sizeof (struct event_videoflush))
    {

        struct event_videoflush *videoflush = data;

        interface->flush(videoflush->x, videoflush->y, videoflush->width, videoflush->height);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct video_interface *interface = resource->data;

        switch (event)
        {

        case EVENT_LINK:
            return kernel_linknode(&interface->resource, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->resource, source);

        case EVENT_INFO:
            return oninfo(interface, source);

        case EVENT_VIDEOCMAP:
            return onvideocmap(interface, source, count, data);

        case EVENT_VIDEOCONF:
            return onvideoconf(interface, source, count, data);

        case EVENT_VIDEOFLUSH:
            return onvideoflush(interface, source, count, data);

        }

    }

    return MESSAGE_FAILED;

}

void video_registerinterface(struct video_interface *interface)
{

    resource_register(&interface->resource);

}

void video_unregisterinterface(struct video_interface *interface)
{

    resource_unregister(&interface->resource);

}

void video_initinterface(struct video_interface *interface, unsigned long framebuffer, unsigned int (*setcmap)(unsigned int count, void *data), void (*setmode)(unsigned int width, unsigned int height, unsigned int bpp))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface, &operands);

    interface->width = 0;
    interface->height = 0;
    interface->bpp = 0;
    interface->framebuffer = framebuffer;
    interface->setcmap = setcmap;
    interface->setmode = setmode;
    interface->flush = 0;

}

void module_init(void)
{

    resource_operands_init(&operands, 0, operands_place, 0);

    dirsrv_init(&dirsrv, RESOURCE_VIDEOINTERFACE);
    service_init(&service, "video");

}

void module_register(void)
{

    service_register(&service, pool_attachnode(&dirsrv.resource));

}

void module_unregister(void)
{

    service_unregister(&service);
    pool_detachnode(&dirsrv.resource);

}

