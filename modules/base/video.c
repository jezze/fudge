#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "video.h"

static struct system_group root;
static struct system_stream data;
static struct system_stream colormap;
static struct system_stream mode;

static unsigned int data_open(struct system_node *self)
{

    return 1;

}

static unsigned int data_close(struct system_node *self)
{

    return 1;

}

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video *interface = (struct base_video *)self->parent;
    struct base_device *device = (struct base_device *)self->parent->parent;

    return interface->read_data(device, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video *interface = (struct base_video *)self->parent;
    struct base_device *device = (struct base_device *)self->parent->parent;

    return interface->write_data(device, offset, count, buffer);

}

static unsigned int colormap_open(struct system_node *self)
{

    return 1;

}

static unsigned int colormap_close(struct system_node *self)
{

    return 1;

}

static unsigned int colormap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video *interface = (struct base_video *)self->parent;
    struct base_device *device = (struct base_device *)self->parent->parent;

    return interface->read_colormap(device, offset, count, buffer);

}

static unsigned int colormap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video *interface = (struct base_video *)self->parent;
    struct base_device *device = (struct base_device *)self->parent->parent;

    return interface->write_colormap(device, offset, count, buffer);

}

static unsigned int mode_open(struct system_node *self)
{

    return 1;

}

static unsigned int mode_close(struct system_node *self)
{

    return 1;

}

static unsigned int mode_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int mode_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_video *interface = (struct base_video *)self->parent;
    struct base_device *device = (struct base_device *)self->parent->parent;

    interface->xres = 320;
    interface->yres = 200;
    interface->bpp = 8;

    interface->mode(device);

    return count;

}

void base_register_video(struct base_video *interface, struct base_device *device)
{

    system_group_add(&root, &device->module.base.node);
    system_group_add(&device->module.base, &interface->base.node);
    system_group_add(&interface->base, &data.node);
    system_group_add(&interface->base, &colormap.node);
    system_group_add(&interface->base, &mode.node);

}

void base_init_video(struct base_video *interface, void (*mode)(struct base_device *device), unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_video));
    system_init_group(&interface->base, "video");

    interface->mode = mode;
    interface->read_data = read_data;
    interface->write_data = write_data;
    interface->read_colormap = read_colormap;
    interface->write_colormap = write_colormap;

}

void base_setup_video()
{

    system_init_group(&root, "video");
    system_init_stream(&data, "data", data_open, data_close, data_read, data_write);
    system_init_stream(&colormap, "colormap", colormap_open, colormap_close, colormap_read, colormap_write);
    system_init_stream(&mode, "mode", mode_open, mode_close, mode_read, mode_write);
    system_register_node(&root.node);

}

