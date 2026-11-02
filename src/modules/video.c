#include <fudge.h>
#include <kernel.h>
#include "video.h"

static struct node_operands operands;
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

        return kernel_place(interface->inode, source, EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo);

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
            return kernel_linknode(&interface->links, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->links, source);

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

struct video_interface *video_findinterface(unsigned int id)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_VIDEOINTERFACE)))
    {

        struct video_interface *interface = current->data;

        if (interface->id == id)
            return interface;

    }

    return 0;

}

void video_registerinterface(struct video_interface *interface)
{

    interface->inode = pool_picknode(&interface->resource, &operands);

    resource_register(&interface->resource);
    service_addchild(&service, interface->inode);

}

void video_unregisterinterface(struct video_interface *interface)
{

    resource_unregister(&interface->resource);
    service_removechild(&service, interface->inode);
    pool_unpicknode(interface->inode);

    interface->inode = 0;

}

void video_initinterface(struct video_interface *interface, unsigned int id, unsigned long framebuffer, unsigned int (*setcmap)(unsigned int count, void *data), void (*setmode)(unsigned int width, unsigned int height, unsigned int bpp))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);
    list_init(&interface->links);

    interface->id = id;
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

    node_operands_init(&operands, 0, operands_place, 0);

    service_init(&service);

}

void module_register(void)
{

    service_registerdirectory(&service, "video");

}

void module_unregister(void)
{

    service_unregisterdirectory(&service);

}

