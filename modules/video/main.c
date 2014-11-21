#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "video.h"

static struct system_node root;

static unsigned int ctrl_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    return memory_read(buffer, count, &node->interface->ctrl, sizeof (struct ctrl_videoctrl), offset);

}

static unsigned int ctrl_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    count = memory_write(&node->interface->ctrl, sizeof (struct ctrl_videoctrl), buffer, count, offset);

    node->interface->setmode(320, 200, 8);

    return count;

}

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    return node->interface->rdata(offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    return node->interface->wdata(offset, count, buffer);

}

static unsigned int colormap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    return node->interface->rcolormap(offset, count, buffer);

}

static unsigned int colormap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interfacenode *node = (struct video_interfacenode *)self->parent;

    return node->interface->wcolormap(offset, count, buffer);

}

void video_registerinterface(struct video_interface *interface)
{

    base_registerinterface(&interface->base);

}

void video_registerinterfacenode(struct video_interfacenode *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->ctrl);
    system_addchild(&node->base, &node->data);
    system_addchild(&node->base, &node->colormap);

}

void video_unregisterinterface(struct video_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void video_unregisterinterfacenode(struct video_interfacenode *node)
{

    system_removechild(&node->base, &node->ctrl);
    system_removechild(&node->base, &node->data);
    system_removechild(&node->base, &node->colormap);
    system_removechild(&root, &node->base);

}

void video_initinterface(struct video_interface *interface, struct base_driver *driver, struct base_bus *bus, unsigned int id, void (*setmode)(unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*rcolormap)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wcolormap)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct video_interface));
    base_initinterface(&interface->base, driver, bus, id);

    interface->setmode = setmode;
    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->rcolormap = rcolormap;
    interface->wcolormap = wcolormap;

}

void video_initinterfacenode(struct video_interfacenode *node, struct video_interface *interface)
{

    memory_clear(node, sizeof (struct video_interfacenode));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&node->colormap, SYSTEM_NODETYPE_NORMAL, "colormap");

    node->interface = interface;
    node->ctrl.read = ctrl_read;
    node->ctrl.write = ctrl_write;
    node->data.read = data_read;
    node->data.write = data_write;
    node->colormap.read = colormap_read;
    node->colormap.write = colormap_write;

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

