#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "video.h"

static struct system_group root;

void video_register_interface(struct video_interface *interface)
{

    system_group_add(&root, &interface->node.root.node);
    system_group_add(&interface->node.root, &interface->node.bpp.node);
    system_group_add(&interface->node.root, &interface->node.xres.node);
    system_group_add(&interface->node.root, &interface->node.yres.node);

}

void video_register_protocol(struct video_protocol *protocol)
{

}

void video_unregister_interface(struct video_interface *interface)
{

    system_group_remove(&interface->node.root, &interface->node.bpp.node);
    system_group_remove(&interface->node.root, &interface->node.xres.node);
    system_group_remove(&interface->node.root, &interface->node.yres.node);
    system_group_remove(&root, &interface->node.root.node);

}

void video_unregister_protocol(struct video_protocol *protocol)
{

}

void video_init_interface(struct video_interface *interface, struct base_driver *driver, void (*enable)(struct video_interface *self), unsigned int (*read)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct video_interface));
    system_init_group(&interface->node.root, driver->module.name);
    system_init_integer(&interface->node.bpp, "bpp", (int *)&interface->bpp);
    system_init_integer(&interface->node.xres, "xres", (int *)&interface->xres);
    system_init_integer(&interface->node.yres, "yres", (int *)&interface->yres);

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

    system_init_group(&root, "video");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

