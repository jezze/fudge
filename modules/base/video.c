#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "video.h"

static struct video_node
{

    struct system_group base;
    struct base_video_interface *interface;
    struct base_device *device;
    struct system_stream data;
    struct system_stream colormap;
    struct system_stream mode;

} node[8];

static struct system_group root;
static struct system_group dev;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_node *node = (struct video_node *)self->parent;

    return node->interface->read_data(node->device, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_node *node = (struct video_node *)self->parent;

    return node->interface->write_data(node->device, offset, count, buffer);

}

static unsigned int colormap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_node *node = (struct video_node *)self->parent;

    return node->interface->read_colormap(node->device, offset, count, buffer);

}

static unsigned int colormap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_node *node = (struct video_node *)self->parent;

    return node->interface->write_colormap(node->device, offset, count, buffer);

}

static unsigned int mode_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_node *node = (struct video_node *)self->parent;

    node->interface->xres = 320;
    node->interface->yres = 200;
    node->interface->bpp = 8;

    node->interface->mode(node->device);

    return count;

}

static unsigned int find_node()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!node[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_node(struct video_node *node, struct base_video_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct video_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->colormap, "colormap");
    system_init_stream(&node->mode, "mode");

    node->interface = interface;
    node->device = device;
    node->data.node.read = data_read;
    node->data.node.write = data_write;
    node->colormap.node.read = colormap_read;
    node->colormap.node.write = colormap_write;
    node->mode.node.write = mode_write;

}

void base_video_register_interface(struct base_video_interface *interface, struct base_device *device)
{

    unsigned int index = find_node();

    if (!index)
        return;

    init_node(&node[index], interface, device);
    system_group_add(&dev, &node[index].base.node);
    system_group_add(&node[index].base, &node[index].data.node);
    system_group_add(&node[index].base, &node[index].colormap.node);
    system_group_add(&node[index].base, &node[index].mode.node);

}

void base_video_init_interface(struct base_video_interface *interface, void (*mode)(struct base_device *device), unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_video_interface));

    interface->mode = mode;
    interface->read_data = read_data;
    interface->write_data = write_data;
    interface->read_colormap = read_colormap;
    interface->write_colormap = write_colormap;

}

void base_video_setup()
{

    memory_clear(node, sizeof (struct video_node) * 8);
    system_init_group(&root, "video");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);

}

