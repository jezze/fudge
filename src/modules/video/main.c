#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "video.h"

static struct system_node root;

void video_notifymode(struct video_interface *interface, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct event_videomode videomode;

    videomode.framebuffer = framebuffer;
    videomode.w = w;
    videomode.h = h;
    videomode.bpp = bpp;

    kernel_notify(&interface->event.links, EVENT_VIDEOMODE, sizeof (struct event_videomode), &videomode);

}

void video_registerinterface(struct video_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->colormap);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

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

void video_initinterface(struct video_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&interface->colormap, SYSTEM_NODETYPE_NORMAL, "colormap");
    system_initnode(&interface->event, SYSTEM_NODETYPE_NORMAL, "event");

    interface->id = id;
    interface->width = 0;
    interface->height = 0;
    interface->bpp = 0;

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

