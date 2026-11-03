#include <fudge.h>
#include <kernel.h>
#include "video.h"

static struct resource_operands operands;
static struct unitsrv unitsrv;
static struct service service;

static unsigned int oninfo(struct video_interface *interface, unsigned int source)
{

    if (interface->framebuffer)
    {

        struct message_videoinfo videoinfo;

        videoinfo.framebuffer = interface->framebuffer;
        videoinfo.width = interface->width;
        videoinfo.height = interface->height;
        videoinfo.bpp = interface->bpp;

        return kernel_place(interface->resource.inode, source, MESSAGE_VIDEOINFO, sizeof (struct message_videoinfo), &videoinfo);

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

    if (count < sizeof (struct message_videoconf))
        return MESSAGE_FAILED;

    if (interface->setmode)
    {

        struct message_videoconf *videoconf = data;

        interface->setmode(videoconf->width, videoconf->height, videoconf->bpp);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

/* only displays that show a copy of the framebuffer need to be told what changed */
static unsigned int onvideoflush(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->flush && count == sizeof (struct message_videoflush))
    {

        struct message_videoflush *videoflush = data;

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

        case MESSAGE_LINK:
            return kernel_linknode(&interface->resource, source);

        case MESSAGE_UNLINK:
            return kernel_unlinknode(&interface->resource, source);

        case MESSAGE_INFO:
            return oninfo(interface, source);

        case MESSAGE_VIDEOCMAP:
            return onvideocmap(interface, source, count, data);

        case MESSAGE_VIDEOCONF:
            return onvideoconf(interface, source, count, data);

        case MESSAGE_VIDEOFLUSH:
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

    unitsrv_init(&unitsrv, RESOURCE_VIDEOINTERFACE);
    service_init(&service, "video");

}

void module_register(void)
{

    service_register(&service, pool_attachnode(&unitsrv.server.resource));

}

void module_unregister(void)
{

    service_unregister(&service);
    pool_detachnode(&unitsrv.server.resource);

}

