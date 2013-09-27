#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

static struct timer_node
{

    struct system_group base;
    struct base_timer_interface *interface;
    struct base_device *device;
    struct system_stream jiffies;

} node[8];

static struct timer_session
{

    struct system_group base;
    struct timer_node *node;
    struct system_stream control;
    struct system_stream sleep;
    char name[8];

} session[8];

static struct system_group root;
static struct system_group dev;
static struct system_stream clone;

static unsigned int sleep_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct timer_session *session = (struct timer_session *)self->parent;

    session->node->interface->add_duration(session->node->device, 3);

    return 0;

}

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct timer_session *session = (struct timer_session *)self->parent;

    return memory_read(buffer, count, session->name, string_length(session->name), offset);

}

static unsigned int find_session()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!session[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_session(struct timer_session *session, unsigned int id, struct timer_node *node)
{

    memory_clear(session, sizeof (struct timer_session));
    memory_write_number(session->name, 8, id, 10, 0);
    system_init_group(&session->base, session->name);
    system_init_stream(&session->control, "control");
    system_init_stream(&session->sleep, "sleep");

    session->node = node;
    session->control.node.read = control_read;
    session->sleep.node.write = sleep_write;

}

static unsigned int clone_open(struct system_node *self)
{

    unsigned int index = find_session();

    if (!index)
        return 0;

    init_session(&session[index], index, &node[1]);
    system_group_add(&root, &session[index].base.node);
    system_group_add(&session[index].base, &session[index].control.node);
    system_group_add(&session[index].base, &session[index].sleep.node);

    return (unsigned int)&session[index].control;

}

static unsigned int jiffies_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct timer_node *node = (struct timer_node *)self->parent;
    char num[32];

    return memory_read(buffer, count, num, memory_write_number(num, 32, node->interface->jiffies, 10, 0), offset);

}

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

static void init_node(struct timer_node *node, struct base_timer_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct timer_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->jiffies, "jiffies");

    node->interface = interface;
    node->device = device;
    node->jiffies.node.read = jiffies_read;

}

void base_timer_register_interface(struct base_timer_interface *interface, struct base_device *device)
{

    unsigned int index = find_node();

    if (!index)
        return;

    init_node(&node[index], interface, device);
    system_group_add(&dev, &node[index].base.node);
    system_group_add(&node[index].base, &node[index].jiffies.node);

}

void base_timer_init_interface(struct base_timer_interface *interface, void (*add_duration)(struct base_device *device, unsigned int duration))
{

    memory_clear(interface, sizeof (struct base_timer_interface));

    interface->add_duration = add_duration;

}

void base_timer_setup()
{

    memory_clear(node, sizeof (struct timer_node) * 8);
    memory_clear(session, sizeof (struct timer_session) * 8);
    system_init_group(&root, "timer");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_init_stream(&clone, "clone");
    system_group_add(&root, &clone.node);

    clone.node.open = clone_open;

}

