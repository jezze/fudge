#include <fudge.h>
#include <kernel.h>
#include "video.h"

static unsigned int onvideocmap(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    return interface->onvideocmap(source, count, data);

}

static unsigned int onvideoconf(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct event_videoconf *videoconf = data;

    return interface->onvideoconf(source, videoconf->width, videoconf->height, videoconf->bpp);

}

static unsigned int place(unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct video_interface *interface = (struct video_interface *)id;

    switch (event)
    {

    case EVENT_VIDEOCMAP:
        return onvideocmap(interface, source, count, data);

    case EVENT_VIDEOCONF:
        return onvideoconf(interface, source, count, data);

    case EVENT_LINK:
        return kernel_link(interface->ichannel, source, interface->ichannel);

    case EVENT_UNLINK:
        return kernel_unlink(interface->ichannel, source);

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

    kernel_notify(interface->ichannel, EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo);

}

void video_registerinterface(struct video_interface *interface)
{

    resource_register(&interface->resource);
    kernel_announce(interface->ichannel, (unsigned int)interface, place);

}

void video_unregisterinterface(struct video_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(interface->ichannel);

}

void video_initinterface(struct video_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*onvideocmap)(unsigned int source, unsigned int count, void *data), unsigned int (*onvideoconf)(unsigned int source, unsigned int width, unsigned int height, unsigned int bpp))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);

    interface->id = id;
    interface->ichannel = ichannel;
    interface->width = 0;
    interface->height = 0;
    interface->bpp = 0;
    interface->onvideocmap = onvideocmap;
    interface->onvideoconf = onvideoconf;

}

