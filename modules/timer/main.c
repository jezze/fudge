#include <fudge/module.h>
#include <base/base.h>
#include <system/system.h>
#include "timer.h"

struct timer_interface_group
{

    struct system_group base;
    struct timer_interface *interface;

};

static struct system_group root;
static struct timer_interface_group interfaces[32];
static unsigned int ninterfaces;

void timer_register_interface(struct timer_interface *interface)
{

    struct timer_interface_group *group = &interfaces[ninterfaces];

    group->interface = interface;

    system_init_group(&group->base, interface->driver->module.name);
    system_group_add(&root, &group->base.node);

    ninterfaces++;

}

void timer_register_protocol(struct timer_protocol *protocol)
{

}

void timer_unregister_interface(struct timer_interface *interface)
{

}

void timer_unregister_protocol(struct timer_protocol *protocol)
{

}

void timer_init_interface(struct timer_interface *interface, struct base_driver *driver)
{

    memory_clear(interface, sizeof (struct timer_interface));

    interface->driver = driver;

}

void timer_init_protocol(struct timer_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct timer_protocol));

    protocol->name = name;

}

void init()
{

    ninterfaces = 0;

    system_init_group(&root, "timer");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

