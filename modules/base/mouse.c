#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "mouse.h"

static struct mouse_node
{

    struct system_group base;
    struct base_mouse_interface *interface;
    struct base_device *device;

} node[8];

static struct mouse_session
{

    struct system_group base;
    struct mouse_node *node;
    struct system_group device;
    struct system_stream control;
    char name[8];

} session[8];

static struct system_group root;
static struct system_group dev;
static struct system_stream clone;

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct mouse_session *session = (struct mouse_session *)self->parent;

    return memory_read(buffer, count, session->name, string_length(session->name), offset);

}

static unsigned int device_open(struct system_node *self)
{

    struct mouse_session *session = (struct mouse_session *)self->parent;

    return session->node->base.node.open(&session->node->base.node);

}

static unsigned int device_walk(struct system_node *self, unsigned int count, const char *path)
{

    struct mouse_session *session = (struct mouse_session *)self->parent;

    return session->node->base.node.walk(&session->node->base.node, count, path);

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

static void init_session(struct mouse_session *session, unsigned int id, struct mouse_node *node)
{

    memory_clear(session, sizeof (struct mouse_session));
    memory_write_number(session->name, 8, id, 10, 0);
    system_init_group(&session->base, session->name);
    system_init_group(&session->device, "device");
    system_init_stream(&session->control, "control");

    session->node = node;
    session->device.node.open = device_open;
    session->device.node.walk = device_walk;
    session->control.node.read = control_read;

}

static unsigned int clone_open(struct system_node *self)
{

    unsigned int index = find_session();

    if (!index)
        return 0;

    init_session(&session[index], index, &node[1]);
    system_group_add(&root, &session[index].base.node);
    system_group_add(&session[index].base, &session[index].device.node);
    system_group_add(&session[index].base, &session[index].control.node);

    return (unsigned int)&session[index].control;

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

    memory_clear(node, sizeof (struct mouse_node) * 8);
    memory_clear(session, sizeof (struct mouse_session) * 8);
    system_init_group(&root, "mouse");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_init_stream(&clone, "clone");
    system_group_add(&root, &clone.node);

    clone.node.open = clone_open;

}

