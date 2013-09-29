#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "keyboard.h"

static struct keyboard_node
{

    struct system_group base;
    struct base_keyboard_interface *interface;
    struct base_device *device;
    struct system_stream data;
    struct system_stream keymap;

} node[8];

static struct keyboard_session
{

    struct system_group base;
    struct keyboard_node *node;
    struct system_group device;
    struct system_stream control;
    char name[8];

} session[8];

static struct system_group root;
static struct system_group dev;
static struct system_stream clone;

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_session *session = (struct keyboard_session *)self->parent;

    return memory_read(buffer, count, session->name, string_length(session->name), offset);

}

static unsigned int device_open(struct system_node *self)
{

    struct keyboard_session *session = (struct keyboard_session *)self->parent;

    return session->node->base.node.open(&session->node->base.node);

}

static unsigned int device_walk(struct system_node *self, unsigned int count, const char *path)
{

    struct keyboard_session *session = (struct keyboard_session *)self->parent;

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

static void init_session(struct keyboard_session *session, unsigned int id, struct keyboard_node *node)
{

    memory_clear(session, sizeof (struct keyboard_session));
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

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_node *node = (struct keyboard_node *)self->parent;

    return node->interface->read_data(node->device, offset, count, buffer);

}

static unsigned int keymap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_node *node = (struct keyboard_node *)self->parent;

    return memory_read(buffer, count, node->interface->keymap, 256, offset);

}

static unsigned int keymap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_node *node = (struct keyboard_node *)self->parent;

    return memory_write(node->interface->keymap, 256, buffer, count, offset);

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

static void init_node(struct keyboard_node *node, struct base_keyboard_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct keyboard_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->keymap, "keymap");

    node->interface = interface;
    node->device = device;
    node->data.node.read = data_read;
    node->keymap.node.read = keymap_read;
    node->keymap.node.write = keymap_write;

}

void base_keyboard_register_interface(struct base_keyboard_interface *interface, struct base_device *device)
{

    unsigned int index = find_node();

    if (!index)
        return;

    init_node(&node[index], interface, device);
    system_group_add(&dev, &node[index].base.node);
    system_group_add(&node[index].base, &node[index].data.node);
    system_group_add(&node[index].base, &node[index].keymap.node);

}

void base_keyboard_init_interface(struct base_keyboard_interface *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_keyboard_interface));

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void base_keyboard_setup()
{

    memory_clear(node, sizeof (struct keyboard_node) * 8);
    memory_clear(session, sizeof (struct keyboard_session) * 8);
    system_init_group(&root, "keyboard");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_init_stream(&clone, "clone");
    system_group_add(&root, &clone.node);

    clone.node.open = clone_open;

}

