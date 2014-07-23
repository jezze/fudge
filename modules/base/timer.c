#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

static struct interface_node
{

    struct system_group base;
    struct base_timer_interface *interface;
    struct base_bus *bus;
    unsigned int id;
    struct system_stream jiffies;

} inode[8];

static struct system_group root;

static unsigned int jiffies_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    char num[32];

    return memory_read(buffer, count, num, ascii_write_value(num, 32, node->interface->jiffies, 10, 0), offset);

}

static unsigned int find_inode()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!inode[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_inode(struct interface_node *node, struct base_timer_interface *interface, struct base_bus *bus, unsigned int id)
{

    memory_clear(node, sizeof (struct interface_node));
    system_init_group(&node->base, bus->name);
    system_init_stream(&node->jiffies, "jiffies");

    node->interface = interface;
    node->bus = bus;
    node->id = id;
    node->jiffies.node.read = jiffies_read;

}

void base_timer_register_interface(struct base_timer_interface *interface, struct base_bus *bus, unsigned int id)
{

    unsigned int index = find_inode();

    if (!index)
        return;

    init_inode(&inode[index], interface, bus, id);
    system_group_add(&root, &inode[index].base.node);
    system_group_add(&inode[index].base, &inode[index].jiffies.node);

}

void base_timer_unregister_interface(struct base_timer_interface *interface)
{

}

void base_timer_init_interface(struct base_timer_interface *interface, void (*add_duration)(struct base_bus *bus, unsigned int id, unsigned int duration))
{

    memory_clear(interface, sizeof (struct base_timer_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_OTHER);

    interface->add_duration = add_duration;

}

void base_timer_setup()
{

    memory_clear(inode, sizeof (struct interface_node) * 8);
    system_init_group(&root, "timer");
    system_register_node(&root.node);

}

