#include <fudge/module.h>
#include <base/base.h>
#include <system/system.h>
#include "video.h"

struct video_interface_group
{

    struct system_group base;
    struct video_interface *interface;
    struct system_stream data;
    struct system_stream resolution;

};

static struct system_group root;
static struct video_interface_group interfaces[32];
static unsigned int ninterfaces;

unsigned int data_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface_group *group = (struct video_interface_group *)self->node.parent;

    return group->interface->read(group->interface, offset, count, buffer);

}

unsigned int data_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface_group *group = (struct video_interface_group *)self->node.parent;

    return group->interface->write(group->interface, offset, count, buffer);

}

unsigned int resolution_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_interface_group *group = (struct video_interface_group *)self->node.parent;
    const void *args[3];

    if (offset > 0)
        return 0;

    args[0] = &group->interface->xres;
    args[1] = &group->interface->yres;
    args[2] = &group->interface->bpp;

    return string_write_format(buffer, count, "%ux%ux%u\n", args);

}

unsigned int resolution_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void video_register_interface(struct video_interface *interface)
{

    struct video_interface_group *group = &interfaces[ninterfaces];

    group->interface = interface;

    system_init_group(&group->base, interface->driver->module.name);
    system_group_add(&root, &group->base.node);
    system_init_stream(&group->data, "data", data_read, data_write);
    system_group_add(&group->base, &group->data.node);
    system_init_stream(&group->resolution, "resolution", resolution_read, resolution_write);
    system_group_add(&group->base, &group->resolution.node);

    ninterfaces++;

}

void video_register_protocol(struct video_protocol *protocol)
{

}

void video_unregister_interface(struct video_interface *interface)
{

}

void video_unregister_protocol(struct video_protocol *protocol)
{

}

void video_init_interface(struct video_interface *interface, struct base_driver *driver, void (*enable)(struct video_interface *self), unsigned int (*read)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct video_interface));

    interface->driver = driver;
    interface->enable = enable;
    interface->read = read;
    interface->write = write;

}

void video_init_protocol(struct video_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct video_protocol));

    protocol->name = name;

}

void init()
{

    ninterfaces = 0;

    system_init_group(&root, "video");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

