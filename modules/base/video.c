#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "video.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    return node->interface->rdata(offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    return node->interface->wdata(offset, count, buffer);

}

static unsigned int colormap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    return node->interface->rcolormap(offset, count, buffer);

}

static unsigned int colormap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    return node->interface->wcolormap(offset, count, buffer);

}

static unsigned int info_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int mode_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    node->interface->setmode(320, 200, 8);

    return count;

}

void base_video_registerinterface(struct base_video_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_video_registernode(struct base_video_node *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);
    system_addchild(&node->base, &node->colormap);
    system_addchild(&node->base, &node->info);
    system_addchild(&node->base, &node->mode);

}

void base_video_unregisterinterface(struct base_video_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_video_unregisternode(struct base_video_node *node)
{

}

void base_video_initinterface(struct base_video_interface *interface, struct base_bus *bus, unsigned int id, void (*setmode)(unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*rcolormap)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wcolormap)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_video_interface));
    base_initinterface(&interface->base, BASE_INTERFACE_TYPE_VIDEO, bus, id);

    interface->setmode = setmode;
    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->rcolormap = rcolormap;
    interface->wcolormap = wcolormap;

}

void base_video_initnode(struct base_video_node *node, struct base_video_interface *interface)
{

    memory_clear(node, sizeof (struct base_video_node));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&node->colormap, SYSTEM_NODETYPE_NORMAL, "colormap");
    system_initnode(&node->info, SYSTEM_NODETYPE_NORMAL, "info");
    system_initnode(&node->mode, SYSTEM_NODETYPE_NORMAL, "mode");

    node->interface = interface;
    node->data.read = data_read;
    node->data.write = data_write;
    node->colormap.read = colormap_read;
    node->colormap.write = colormap_write;
    node->info.read = info_read;
    node->mode.write = mode_write;

}

void base_video_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "video");
    system_registernode(&root);

}

