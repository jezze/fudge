#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "video.h"

static struct system_group root;
static struct system_stream data;
static struct system_stream colormap;
static struct system_stream mode;

static unsigned int data_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->node.parent;
    struct base_device *device = (struct base_device *)interface->base.node.parent;

    return interface->read_data(device, offset, count, buffer);

}

static unsigned int data_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->node.parent;
    struct base_device *device = (struct base_device *)interface->base.node.parent;

    return interface->write_data(device, offset, count, buffer);

}

static unsigned int colormap_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->node.parent;
    struct base_device *device = (struct base_device *)interface->base.node.parent;

    return interface->read_colormap(device, offset, count, buffer);

}

static unsigned int colormap_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->node.parent;
    struct base_device *device = (struct base_device *)interface->base.node.parent;

    return interface->write_colormap(device, offset, count, buffer);

}

static unsigned int mode_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int mode_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface *interface = (struct video_interface *)self->node.parent;
    struct base_device *device = (struct base_device *)interface->base.node.parent;

    interface->xres = 320;
    interface->yres = 200;
    interface->bpp = 8;

    interface->mode(device);

    return count;

}

void video_register_device(struct video_interface *interface, struct base_device *device)
{

    system_group_add(&root, &device->module.base.node);
    system_group_add(&device->module.base, &interface->base.node);
    system_group_add(&interface->base, &data.node);
    system_group_add(&interface->base, &colormap.node);
    system_group_add(&interface->base, &mode.node);

}

void video_init_interface(struct video_interface *interface, void (*mode)(struct base_device *device), unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct video_interface));
    system_init_group(&interface->base, "video");

    interface->mode = mode;
    interface->read_data = read_data;
    interface->write_data = write_data;
    interface->read_colormap = read_colormap;
    interface->write_colormap = write_colormap;

}

void init()
{

    system_init_group(&root, "video");
    system_init_stream(&data, "data", data_read, data_write);
    system_init_stream(&colormap, "colormap", colormap_read, colormap_write);
    system_init_stream(&mode, "mode", mode_read, mode_write);
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

