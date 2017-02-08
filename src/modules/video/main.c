#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "video.h"

static struct system_node root;
static struct system_node event;
static struct list eventlinks;

void video_notifymode(unsigned int w, unsigned int h, unsigned int bpp)
{

    struct {struct event_header header; struct event_videomode videomode;} message;

    message.header.type = EVENT_VIDEOMODE;
    message.header.source = 0;
    message.header.destination = 0;
    message.videomode.w = w;
    message.videomode.h = h;
    message.videomode.bpp = bpp;

    event_multicast(&eventlinks, &message.header, sizeof (struct event_header) + sizeof (struct event_videomode));

}

static unsigned int event_open(struct system_node *self, struct service_state *state)
{

    list_add(&eventlinks, &state->link);

    return state->id;

}

static unsigned int event_close(struct system_node *self, struct service_state *state)
{

    list_remove(&eventlinks, &state->link);

    return state->id;

}

static unsigned int event_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return task_read(state->link.data, buffer, count);

}

static unsigned int event_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return event_send(&eventlinks, state, buffer, count);

}

static unsigned int interfacectrl_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct video_interface *interface = self->resource->data;

    return memory_read(buffer, count, &interface->settings, sizeof (struct ctrl_videosettings), state->offset);

}

static unsigned int interfacectrl_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct video_interface *interface = self->resource->data;

    count = memory_write(&interface->settings, sizeof (struct ctrl_videosettings), buffer, count, state->offset);

    interface->setmode(&interface->settings);

    return count;

}

static unsigned int interfacedata_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct video_interface *interface = self->resource->data;

    return interface->rdata(state->offset, buffer, count);

}

static unsigned int interfacedata_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct video_interface *interface = self->resource->data;

    return interface->wdata(state->offset, buffer, count);

}

static unsigned int interfacecolormap_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct video_interface *interface = self->resource->data;

    return interface->rcolormap(state->offset, buffer, count);

}

static unsigned int interfacecolormap_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct video_interface *interface = self->resource->data;

    return interface->wcolormap(state->offset, buffer, count);

}

void video_registerinterface(struct video_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->colormap);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void video_unregisterinterface(struct video_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->colormap);
    system_removechild(&root, &interface->root);

}

void video_initinterface(struct video_interface *interface, void (*setmode)(struct ctrl_videosettings *settings), unsigned int (*rdata)(unsigned int offset, void *buffer, unsigned int count), unsigned int (*wdata)(unsigned int offset, void *buffer, unsigned int count), unsigned int (*rcolormap)(unsigned int offset, void *buffer, unsigned int count), unsigned int (*wcolormap)(unsigned int offset, void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data", &interface->resource);
    system_initresourcenode(&interface->colormap, SYSTEM_NODETYPE_NORMAL, "colormap", &interface->resource);

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

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "video");
    system_initnode(&event, SYSTEM_NODETYPE_NORMAL, "event");

    event.open = event_open;
    event.close = event_close;
    event.read = event_read;
    event.write = event_write;

    system_addchild(&root, &event);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

