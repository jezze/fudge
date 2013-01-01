#include <fudge/memory.h>
#include <base/base.h>
#include <system/system.h>
#include "video.h"

static struct system_group root;

void video_register_interface(struct video_interface *interface)
{

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

void video_init_interface(struct video_interface *interface, struct base_driver *driver, void (*enable)(struct video_interface *self), unsigned int (*read_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct video_interface));

    interface->driver = driver;
    interface->enable = enable;
    interface->read_data = read_data;
    interface->write_data = write_data;

}

void video_init_protocol(struct video_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct video_protocol));

    protocol->name = name;

}

void init()
{

    system_init_group(&root, "video");

    /*
    system_register_node(&root.base);
    */

}

void destroy()
{

    system_unregister_node(&root.base);

}

