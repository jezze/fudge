#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "video.h"

struct video_group
{

    struct system_group base;
    struct base_video *interface;
    struct base_device *device;
    struct system_stream data;
    struct system_stream colormap;
    struct system_stream mode;

};

static struct system_group root;
static struct system_group dev;
static struct video_group groups[8];

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_group *group = (struct video_group *)self->parent;

    return group->interface->read_data(group->device, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_group *group = (struct video_group *)self->parent;

    return group->interface->write_data(group->device, offset, count, buffer);

}

static unsigned int colormap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_group *group = (struct video_group *)self->parent;

    return group->interface->read_colormap(group->device, offset, count, buffer);

}

static unsigned int colormap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_group *group = (struct video_group *)self->parent;

    return group->interface->write_colormap(group->device, offset, count, buffer);

}

static unsigned int mode_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_group *group = (struct video_group *)self->parent;

    group->interface->xres = 320;
    group->interface->yres = 200;
    group->interface->bpp = 8;

    group->interface->mode(group->device);

    return count;

}

static unsigned int find_group()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!groups[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_group(struct video_group *group, struct base_video *interface, struct base_device *device)
{

    memory_clear(group, sizeof (struct video_group));
    system_init_group(&group->base, device->module.name);
    system_init_stream(&group->data, "data");
    system_init_stream(&group->colormap, "colormap");
    system_init_stream(&group->mode, "mode");

    group->interface = interface;
    group->device = device;

    group->data.node.read = data_read;
    group->data.node.write = data_write;
    group->colormap.node.read = colormap_read;
    group->colormap.node.write = colormap_write;
    group->mode.node.write = mode_write;

}

void base_register_video(struct base_video *interface, struct base_device *device)
{

    unsigned int index = find_group();

    if (!index)
        return;

    init_group(&groups[index], interface, device);
    system_group_add(&groups[index].base, &groups[index].data.node);
    system_group_add(&groups[index].base, &groups[index].colormap.node);
    system_group_add(&groups[index].base, &groups[index].mode.node);
    system_group_add(&dev, &groups[index].base.node);

}

void base_init_video(struct base_video *interface, void (*mode)(struct base_device *device), unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_video));

    interface->mode = mode;
    interface->read_data = read_data;
    interface->write_data = write_data;
    interface->read_colormap = read_colormap;
    interface->write_colormap = write_colormap;

}

void base_setup_video()
{

    system_init_group(&root, "video");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

