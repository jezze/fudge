#include <fudge/module.h>
#include <base/base.h>
#include <system/system.h>
#include "timer.h"

static struct system_group root;

void timer_register_interface(struct timer_interface *interface)
{

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

    system_init_group(&root, "timer");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

