#include <fudge.h>
#include <kernel.h>
#include "video.h"

static struct service service;

static struct node *service_match(unsigned int count, char *name)
{

    if (count == 2 && buffer_match(name, ":", 1))
    {

        struct resource *current = 0;
        unsigned int index = cstring_toint(name[1]);
        unsigned int i;

        for (i = 0; (current = resource_foreachtype(current, RESOURCE_VIDEOINTERFACE)); i++)
        {

            struct video_interface *interface = current->data;

            if (i == index)
                return &interface->node;

        }

    }

    return 0;

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

static unsigned int place(struct node *source, struct node *target, unsigned int ichannel, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_VIDEOCMAP:
        return onvideocmap(target->interface, source->ichannel, count, data);

    case EVENT_VIDEOCONF:
        return onvideoconf(target->interface, source->ichannel, count, data);

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

    kernel_notify(&interface->node, EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo);

}

void video_registerinterface(struct video_interface *interface)
{

    resource_register(&interface->resource);
    kernel_announce(&interface->node);

}

void video_unregisterinterface(struct video_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(&interface->node);

}

void video_initinterface(struct video_interface *interface, unsigned int id, unsigned int (*onvideocmap)(unsigned int source, unsigned int count, void *data), unsigned int (*onvideoconf)(unsigned int source, unsigned int width, unsigned int height, unsigned int bpp))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);
    node_init(&interface->node, interface, place);

    interface->id = id;
    interface->width = 0;
    interface->height = 0;
    interface->bpp = 0;
    interface->onvideocmap = onvideocmap;
    interface->onvideoconf = onvideoconf;

}

void module_init(void)
{

    service_init(&service, "video", service_match);
    service_register(&service);

}

