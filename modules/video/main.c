#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include "video.h"

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = self->resource->data;
    struct ctrl_videosettings settings;

    settings.header.type = CTRL_TYPE_VIDEO;
    settings.w = interface->w;
    settings.h = interface->h;
    settings.bpp = interface->bpp;

    return memory_read(buffer, count, &settings, sizeof (struct ctrl_videosettings), offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = self->resource->data;

    interface->setmode(320, 200, 8);

    return 0;

}

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = self->resource->data;

    return interface->rdata(offset, count, buffer);

}

static unsigned int interfacenode_datawrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = self->resource->data;

    return interface->wdata(offset, count, buffer);

}

static unsigned int interfacenode_colormapread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = self->resource->data;

    return interface->rcolormap(offset, count, buffer);

}

static unsigned int interfacenode_colormapwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = self->resource->data;

    return interface->wcolormap(offset, count, buffer);

}

void video_registerinterface(struct video_interface *interface, unsigned int id)
{

    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->colormap);
    system_registernode(&interface->root);

    interface->id = id;

}

void video_unregisterinterface(struct video_interface *interface)
{

    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->colormap);
    system_unregisternode(&interface->root);

}

void video_initinterface(struct video_interface *interface, void (*setmode)(unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*rcolormap)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wcolormap)(unsigned int offset, unsigned int count, void *buffer))
{

    resource_init(&interface->resource, 0, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "video");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&interface->colormap, SYSTEM_NODETYPE_NORMAL, "colormap");

    interface->setmode = setmode;
    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->rcolormap = rcolormap;
    interface->wcolormap = wcolormap;
    interface->ctrl.resource = &interface->resource;
    interface->ctrl.read = interfacenode_ctrlread;
    interface->ctrl.write = interfacenode_ctrlwrite;
    interface->data.resource = &interface->resource;
    interface->data.read = interfacenode_dataread;
    interface->data.write = interfacenode_datawrite;
    interface->colormap.resource = &interface->resource;
    interface->colormap.read = interfacenode_colormapread;
    interface->colormap.write = interfacenode_colormapwrite;

}

void module_init()
{

}

void module_register()
{

}

void module_unregister()
{

}

