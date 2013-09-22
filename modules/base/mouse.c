#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "mouse.h"

static struct mouse_node
{

    struct system_group base;
    struct base_mouse_interface *interface;
    struct base_device *device;

} node[8];

static struct system_group root;
static struct system_group dev;

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

static void init_node(struct mouse_node *node, struct base_mouse_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct mouse_node));
    system_init_group(&node->base, device->module.name);

    node->interface = interface;
    node->device = device;

}

void base_mouse_register_interface(struct base_mouse_interface *interface, struct base_device *device)
{

    unsigned int index = find_node();

    if (!index)
        return;

    init_node(&node[index], interface, device);
    system_group_add(&dev, &node[index].base.node);

}

void base_mouse_init_interface(struct base_mouse_interface *interface)
{

    memory_clear(interface, sizeof (struct base_mouse_interface));

}

void base_mouse_setup()
{

    system_init_group(&root, "mouse");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

