#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include "video.h"

static struct system_node root;

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    return memory_read(buffer, count, &node->interface->settings, sizeof (struct ctrl_videosettings), offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    count = memory_write(&node->interface->settings, sizeof (struct ctrl_videosettings), buffer, count, offset);

    node->interface->setmode(320, 200, 8);

    return count;

}

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    return node->interface->rdata(offset, count, buffer);

}

static unsigned int interfacenode_datawrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    return node->interface->wdata(offset, count, buffer);

}

static unsigned int interfacenode_colormapread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    return node->interface->rcolormap(offset, count, buffer);

}

static unsigned int interfacenode_colormapwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    return node->interface->wcolormap(offset, count, buffer);

}

void video_registerinterface(struct video_interface *interface, struct base_bus *bus, unsigned int id)
{

    base_registerinterface(&interface->base, bus, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.ctrl);
    system_addchild(&interface->node.base, &interface->node.data);
    system_addchild(&interface->node.base, &interface->node.colormap);

}

void video_unregisterinterface(struct video_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.ctrl);
    system_removechild(&interface->node.base, &interface->node.data);
    system_removechild(&interface->node.base, &interface->node.colormap);
    system_removechild(&root, &interface->node.base);

}

void video_initinterface(struct video_interface *interface, struct base_driver *driver, void (*setmode)(unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*rcolormap)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wcolormap)(unsigned int offset, unsigned int count, void *buffer))
{

    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&interface->node.colormap, SYSTEM_NODETYPE_NORMAL, "colormap");
    ctrl_init_videosettings(&interface->settings);

    interface->setmode = setmode;
    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->rcolormap = rcolormap;
    interface->wcolormap = wcolormap;
    interface->node.interface = interface;
    interface->node.ctrl.read = interfacenode_ctrlread;
    interface->node.ctrl.write = interfacenode_ctrlwrite;
    interface->node.data.read = interfacenode_dataread;
    interface->node.data.write = interfacenode_datawrite;
    interface->node.colormap.read = interfacenode_colormapread;
    interface->node.colormap.write = interfacenode_colormapwrite;

}

void init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "video");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

