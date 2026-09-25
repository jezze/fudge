#include <fudge.h>
#include <kernel.h>
#include "video.h"

static struct node_operands operands;

static unsigned int oninfo(struct video_interface *interface, unsigned int source)
{

    if (interface->oninfo)
    {

        struct event_videoinfo videoinfo;

        interface->oninfo(&videoinfo);

        return kernel_place(interface->service.inode, source, EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int onvideocmap(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->onvideocmap)
    {

        interface->onvideocmap(count, data);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int onvideoconf(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->onvideoconf)
    {

        struct event_videoconf *videoconf = data;

        interface->onvideoconf(videoconf->width, videoconf->height, videoconf->bpp);

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
            return kernel_linknode(&interface->service.links, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->service.links, source);

        case EVENT_INFO:
            return oninfo(interface, source);

        case EVENT_VIDEOCMAP:
            return onvideocmap(interface, source, count, data);

        case EVENT_VIDEOCONF:
            return onvideoconf(interface, source, count, data);

        }

    }

    return MESSAGE_FAILED;

}

void video_registerinterface(struct video_interface *interface)
{

    unsigned int inode = pool_picknode(&interface->resource, &operands);

    resource_register(&interface->resource);
    service_register(&interface->service, inode, "video");

}

void video_unregisterinterface(struct video_interface *interface)
{

    resource_unregister(&interface->resource);
    service_unregister(&interface->service);

}

void video_initinterface(struct video_interface *interface, unsigned int id, void (*oninfo)(struct event_videoinfo *videoinfo), unsigned int (*onvideocmap)(unsigned int count, void *data), void (*onvideoconf)(unsigned int width, unsigned int height, unsigned int bpp))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);
    service_init(&interface->service);

    interface->id = id;
    interface->width = 0;
    interface->height = 0;
    interface->bpp = 0;
    interface->oninfo = oninfo;
    interface->onvideocmap = onvideocmap;
    interface->onvideoconf = onvideoconf;

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place);

}

