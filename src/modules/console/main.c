#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "console.h"

void console_notify(struct console_interface *interface, unsigned int count, void *buffer)
{

    system_multicast(&interface->data.links, count, buffer);

}

static unsigned int interfacectrl_read(struct system_node *self, struct service_state *state, unsigned int count, void *buffer)
{

    struct console_interface *interface = self->resource->data;
    struct ctrl_consolesettings *settings = buffer;

    return memory_read(settings, count, &interface->settings, sizeof (struct ctrl_consolesettings), state->offset);

}

static unsigned int interfacedata_write(struct system_node *self, struct service_state *state, unsigned int count, void *buffer)
{

    struct console_interface *interface = self->resource->data;

    return interface->send(count, buffer);

}

void console_registerinterface(struct console_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_registernode(&interface->root);

    interface->id = id;

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->data);
    system_unregisternode(&interface->root);

}

void console_initinterface(struct console_interface *interface, unsigned int (*send)(unsigned int count, void *buffer))
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "console", &interface->resource);
    system_initresourcenode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data", &interface->resource);

    interface->send = send;
    interface->ctrl.read = interfacectrl_read;
    interface->data.write = interfacedata_write;

}

