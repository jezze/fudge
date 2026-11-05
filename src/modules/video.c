#include <fudge.h>
#include <kernel.h>
#include "video.h"

static struct resource_operands operands;
static struct unitsrv unitsrv;

static unsigned int oninfo(struct video_interface *interface, unsigned int source)
{

    if (interface->framebuffer)
    {

        struct message_videoinfo videoinfo;

        videoinfo.framebuffer = interface->framebuffer;
        videoinfo.width = interface->width;
        videoinfo.height = interface->height;
        videoinfo.bpp = interface->bpp;

        return kernel_place(interface->resource.inode, source, MESSAGE_TYPE_VIDEOINFO, sizeof (struct message_videoinfo), &videoinfo);

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

    struct message_videoconf *videoconf = message_validate(MESSAGE_TYPE_VIDEOCONF, data, count);

    if (interface->setmode && videoconf)
    {

        interface->setmode(videoconf->width, videoconf->height, videoconf->bpp);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int onvideoflush(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct message_videoflush *videoflush = message_validate(MESSAGE_TYPE_VIDEOFLUSH, data, count);

    if (interface->flush && videoflush)
    {

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

        case MESSAGE_TYPE_LINK:
            return kernel_linknode(&interface->resource, source);

        case MESSAGE_TYPE_UNLINK:
            return kernel_unlinknode(&interface->resource, source);

        case MESSAGE_TYPE_INFO:
            return oninfo(interface, source);

        case MESSAGE_TYPE_VIDEOCMAP:
            return onvideocmap(interface, source, count, data);

        case MESSAGE_TYPE_VIDEOCONF:
            return onvideoconf(interface, source, count, data);

        case MESSAGE_TYPE_VIDEOFLUSH:
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

    unitsrv_init(&unitsrv, RESOURCE_VIDEOINTERFACE, "video");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

