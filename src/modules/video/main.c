#include <fudge.h>
#include <kernel.h>
#include <event/event.h>
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
    message.videomode.w = w;
    message.videomode.h = h;
    message.videomode.bpp = bpp;

    spinlock_acquire(&interface->eventlock);
    event_multicast(&interface->eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_videomode));
    spinlock_release(&interface->eventlock);

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

static unsigned int interfacedata_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct video_interface *interface = self->resource->data;

    return interface->rdata(offset, buffer, count);

}

static unsigned int interfacedata_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct video_interface *interface = self->resource->data;

    return interface->wdata(offset, buffer, count);

}

static unsigned int interfacecolormap_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct video_interface *interface = self->resource->data;

    return interface->rcolormap(offset, buffer, count);

}

static unsigned int interfacecolormap_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct video_interface *interface = self->resource->data;

    return interface->wcolormap(offset, buffer, count);

}

static unsigned int interfaceevent_open(struct system_node *self, struct service_state *state)
{

    struct video_interface *interface = self->resource->data;

    list_lockadd(&interface->eventstates, &state->item, &interface->eventlock);

    return (unsigned int)self;

}

static unsigned int interfaceevent_close(struct system_node *self, struct service_state *state)
{

    struct video_interface *interface = self->resource->data;

    list_lockremove(&interface->eventstates, &state->item, &interface->eventlock);

    return (unsigned int)self;

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

void video_initinterface(struct video_interface *interface, void (*setmode)(struct ctrl_videosettings *settings), unsigned int (*rdata)(unsigned int offset, void *buffer, unsigned int count), unsigned int (*wdata)(unsigned int offset, void *buffer, unsigned int count), unsigned int (*rcolormap)(unsigned int offset, void *buffer, unsigned int count), unsigned int (*wcolormap)(unsigned int offset, void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data", &interface->resource);
    system_initresourcenode(&interface->colormap, SYSTEM_NODETYPE_NORMAL, "colormap", &interface->resource);
    system_initresourcenode(&interface->event, SYSTEM_NODETYPE_NORMAL, "event", &interface->resource);

    interface->setmode = setmode;
    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->rcolormap = rcolormap;
    interface->wcolormap = wcolormap;
    interface->ctrl.read = interfacectrl_read;
    interface->ctrl.write = interfacectrl_write;
    interface->data.read = interfacedata_read;
    interface->data.write = interfacedata_write;
    interface->colormap.read = interfacecolormap_read;
    interface->colormap.write = interfacecolormap_write;
    interface->event.open = interfaceevent_open;
    interface->event.close = interfaceevent_close;

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

