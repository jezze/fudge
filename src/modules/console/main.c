#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "console.h"

static struct system_node root;

void console_notify(struct console_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicast(&interface->datastates, &interface->datalock, buffer, count);

}

static unsigned int interfacectrl_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct console_interface *interface = self->resource->data;

    return memory_read(buffer, count, &interface->settings, sizeof (struct ctrl_consolesettings), offset);

}

static struct system_node *interfacedata_open(struct system_node *self, struct service_state *state)
{

    struct console_interface *interface = self->resource->data;

    list_lockadd(&interface->datastates, &state->item, &interface->datalock);

    return self;

}

static struct system_node *interfacedata_close(struct system_node *self, struct service_state *state)
{

    struct console_interface *interface = self->resource->data;

    list_lockremove(&interface->datastates, &state->item, &interface->datalock);

    return self;

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
    system_addchild(&interface->root, &interface->idata);
    system_addchild(&interface->root, &interface->odata);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->idata);
    system_removechild(&interface->root, &interface->odata);
    system_removechild(&root, &interface->root);

}

void console_initinterface(struct console_interface *interface, unsigned int (*send)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &interface->resource);
    system_initresourcenode(&interface->idata, SYSTEM_NODETYPE_MAILBOX, "idata", &interface->resource);
    system_initresourcenode(&interface->odata, SYSTEM_NODETYPE_NORMAL, "odata", &interface->resource);

    interface->send = send;
    interface->ctrl.read = interfacectrl_read;
    interface->idata.open = interfacedata_open;
    interface->idata.close = interfacedata_close;
    interface->odata.write = interfacedata_write;

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

