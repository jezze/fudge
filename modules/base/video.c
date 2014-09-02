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

    return node->interface->rdata(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    return node->interface->wdata(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int colormap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    return node->interface->rcolormap(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int colormap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    return node->interface->wcolormap(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int info_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int mode_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    node->interface->setmode(node->device->bus, node->device->id, 320, 200, 8);

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

void base_video_initinterface(struct base_video_interface *interface, void (*setmode)(struct base_bus *bus, unsigned int id, unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*rdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*rcolormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*wcolormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_video_interface));
    base_initinterface(&interface->base, BASE_INTERFACE_TYPE_VIDEO);

    interface->setmode = setmode;
    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->rcolormap = rcolormap;
    interface->wcolormap = wcolormap;

}

void base_video_initnode(struct base_video_node *node, struct base_device *device, struct base_video_interface *interface)
{

    memory_clear(node, sizeof (struct base_video_node));
    system_initmultigroup(&node->base, device->bus->name);
    system_initstream(&node->data, "data");
    system_initstream(&node->colormap, "colormap");
    system_initstream(&node->info, "info");
    system_initstream(&node->mode, "mode");

    node->device = device;
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

    system_initgroup(&root, "video");
    system_registernode(&root);

}

