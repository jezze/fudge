#include <fudge.h>
#include <kernel.h>
#include <event/base.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "video.h"

static struct system_node root;

void video_notifymode(struct video_interface *interface, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct {struct event_header header; struct event_videomode videomode;} message;

    message.header.type = EVENT_VIDEOMODE;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.header.length = sizeof (message);
    message.videomode.w = w;
    message.videomode.h = h;
    message.videomode.bpp = bpp;

    event_multicast(&interface->event.states, &message.header);

}

static unsigned int interfacectrl_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct video_interface *interface = self->resource->data;

    return memory_read(buffer, count, &interface->settings, sizeof (struct ctrl_videosettings), offset);

}

static unsigned int interfacectrl_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct video_interface *interface = self->resource->data;

    count = memory_write(&interface->settings, sizeof (struct ctrl_videosettings), buffer, count, offset);

    interface->setmode(&interface->settings);

    return count;

}

void video_registerinterface(struct video_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->colormap);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

    interface->id = id;

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

void video_initinterface(struct video_interface *interface, void (*setmode)(struct ctrl_videosettings *settings))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data", &interface->resource);
    system_initresourcenode(&interface->colormap, SYSTEM_NODETYPE_NORMAL, "colormap", &interface->resource);
    system_initresourcenode(&interface->event, SYSTEM_NODETYPE_MAILBOX, "event", &interface->resource);

    interface->setmode = setmode;
    interface->ctrl.read = interfacectrl_read;
    interface->ctrl.write = interfacectrl_write;

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

