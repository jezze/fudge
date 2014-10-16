#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

static struct system_node root;

static unsigned int sleep_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_timer_node *node = (struct base_timer_node *)self->parent;

    /* 10s */
    node->interface->sleep(1000);

    return 0;

}

void base_timer_registerinterface(struct base_timer_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_timer_registernode(struct base_timer_node *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->sleep);

}

void base_timer_unregisterinterface(struct base_timer_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_timer_unregisternode(struct base_timer_node *node)
{

}

void base_timer_initinterface(struct base_timer_interface *interface, struct base_bus *bus, unsigned int id, void (*sleep)(unsigned int duration))
{

    memory_clear(interface, sizeof (struct base_timer_interface));
    base_initinterface(&interface->base, bus, id);

    interface->sleep = sleep;

}

void base_timer_initnode(struct base_timer_node *node, struct base_timer_interface *interface)
{

    memory_clear(node, sizeof (struct base_timer_node));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->sleep, SYSTEM_NODETYPE_NORMAL, "sleep");

    node->interface = interface;
    node->sleep.read = sleep_read;

}

void base_timer_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "timer");
    system_registernode(&root);

}

