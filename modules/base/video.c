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

    return node->interface->read_data(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    return node->interface->write_data(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int colormap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    return node->interface->read_colormap(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int colormap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    return node->interface->write_colormap(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int info_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int mode_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video_node *node = (struct base_video_node *)self->parent;

    node->interface->set_mode(node->device->bus, node->device->id, 320, 200, 8);

    return count;

}

void base_video_register_interface(struct base_video_interface *interface)
{

    base_register_interface(&interface->base);

}

void base_video_register_node(struct base_video_node *node)
{

    system_add_child(&root, &node->base);
    system_add_child(&node->base, &node->data);
    system_add_child(&node->base, &node->colormap);
    system_add_child(&node->base, &node->info);
    system_add_child(&node->base, &node->mode);

}

void base_video_unregister_interface(struct base_video_interface *interface)
{

    base_unregister_interface(&interface->base);

}

void base_video_unregister_node(struct base_video_node *node)
{

}

void base_video_init_interface(struct base_video_interface *interface, void (*set_mode)(struct base_bus *bus, unsigned int id, unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_colormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_colormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_video_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_VIDEO);

    interface->set_mode = set_mode;
    interface->read_data = read_data;
    interface->write_data = write_data;
    interface->read_colormap = read_colormap;
    interface->write_colormap = write_colormap;

}

void base_video_init_node(struct base_video_node *node, struct base_device *device, struct base_video_interface *interface)
{

    memory_clear(node, sizeof (struct base_video_node));
    system_init_multigroup(&node->base, device->bus->name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->colormap, "colormap");
    system_init_stream(&node->info, "info");
    system_init_stream(&node->mode, "mode");

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

    system_init_group(&root, "video");
    system_register_node(&root);

}

