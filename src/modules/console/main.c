#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "console.h"

static struct system_node root;

void console_notify(struct console_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicast(&interface->datastates, buffer, count);

}

static unsigned int interfacectrl_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct console_interface *interface = self->resource->data;

    return memory_read(buffer, count, &interface->settings, sizeof (struct ctrl_consolesettings), offset);

}

static unsigned int interfacedata_open(struct system_node *self, struct service_state *state)
{

    struct console_interface *interface = self->resource->data;

    list_add(&interface->datastates, &state->item);

    return (unsigned int)self;

}

static unsigned int interfacedata_close(struct system_node *self, struct service_state *state)
{

    struct console_interface *interface = self->resource->data;

    list_remove(&interface->datastates, &state->item);

    return (unsigned int)self;

}

static unsigned int interfacedata_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct console_interface *interface = self->resource->data;

    return interface->send(buffer, count);

}

void console_registerinterface(struct console_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&root, &interface->root);

}

void console_initinterface(struct console_interface *interface, unsigned int (*send)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data", &interface->resource);

    interface->send = send;
    interface->ctrl.read = interfacectrl_read;
    interface->data.open = interfacedata_open;
    interface->data.close = interfacedata_close;
    interface->data.read = system_readtaskmailbox;
    interface->data.write = interfacedata_write;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "console");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

