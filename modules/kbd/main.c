#include <base/base.h>
#include <system/system.h>
#include "kbd.h"

static struct system_group root;

void kbd_register_interface(struct kbd_interface *interface)
{

    system_init_group(&interface->node.root, interface->driver->module.name);
    system_init_string(&interface->node.info, "testing", "this seem to be working");
    system_init_integer(&interface->node.size, "size", 0x4ABC8);

    system_group_add(&root, &interface->node.root.base);
    system_group_add(&interface->node.root, &interface->node.info.base);
    system_group_add(&interface->node.root, &interface->node.size.base);

}

void kbd_register_protocol(struct kbd_protocol *protocol)
{

}

void kbd_unregister_interface(struct kbd_interface *interface)
{

    system_group_remove(&interface->node.root, &interface->node.size.base);
    system_group_remove(&interface->node.root, &interface->node.info.base);
    system_group_remove(&root, &interface->node.root.base);

}

void kbd_unregister_protocol(struct kbd_protocol *protocol)
{

}

void init()
{

    system_init_group(&root, "kbd");
    system_register_node(&root.base);

}

void destroy()
{

    system_unregister_node(&root.base);

}

