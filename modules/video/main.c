#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include "video.h"

static struct system_node root;

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->parent;

    return memory_read(buffer, count, &interface->settings, sizeof (struct ctrl_videosettings), offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->parent;

    count = memory_write(&interface->settings, sizeof (struct ctrl_videosettings), buffer, count, offset);

    interface->setmode(320, 200, 8);

    return count;

}

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->parent;

    return interface->rdata(offset, count, buffer);

}

static unsigned int interfacenode_datawrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->parent;

    return interface->wdata(offset, count, buffer);

}

static unsigned int interfacenode_colormapread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->parent;

    return interface->rcolormap(offset, count, buffer);

}

static unsigned int interfacenode_colormapwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->parent;

    return interface->wcolormap(offset, count, buffer);

}

void video_registerinterface(struct video_interface *interface, unsigned int id)
{

    system_registerinterface(&interface->base, id);
    system_addchild(&interface->base.root, &interface->ctrl);
    system_addchild(&interface->base.root, &interface->data);
    system_addchild(&interface->base.root, &interface->colormap);
    system_addchild(&root, &interface->base.root);

}

void video_unregisterinterface(struct video_interface *interface)
{

    system_unregisterinterface(&interface->base);
    system_removechild(&interface->base.root, &interface->ctrl);
    system_removechild(&interface->base.root, &interface->data);
    system_removechild(&interface->base.root, &interface->colormap);
    system_removechild(&root, &interface->base.root);

}

void video_initinterface(struct video_interface *interface, struct base_driver *driver, void (*setmode)(unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*rcolormap)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wcolormap)(unsigned int offset, unsigned int count, void *buffer))
{

    system_initinterface(&interface->base, driver);
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&interface->colormap, SYSTEM_NODETYPE_NORMAL, "colormap");
    ctrl_init_videosettings(&interface->settings);

    interface->setmode = setmode;
    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->rcolormap = rcolormap;
    interface->wcolormap = wcolormap;
    interface->ctrl.read = interfacenode_ctrlread;
    interface->ctrl.write = interfacenode_ctrlwrite;
    interface->data.read = interfacenode_dataread;
    interface->data.write = interfacenode_datawrite;
    interface->colormap.read = interfacenode_colormapread;
    interface->colormap.write = interfacenode_colormapwrite;

}

void module_init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "video");

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

