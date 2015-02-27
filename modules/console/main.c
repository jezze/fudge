#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <event/event.h>
#include "console.h"

static struct system_node root;

void console_notify(struct console_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->in.mailboxes, count, buffer);
    event_notify(EVENT_TYPE_CONSOLE, count, buffer);

}

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int interfacenode_outwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interface *interface = self->resource->data;

    return interface->send(offset, count, buffer);

}

void console_registerinterface(struct console_interface *interface, unsigned int id)
{

    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->in);
    system_addchild(&interface->root, &interface->out);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void console_unregisterinterface(struct console_interface *interface)
{

    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->in);
    system_removechild(&interface->root, &interface->out);
    system_removechild(&root, &interface->root);

}

void console_initinterface(struct console_interface *interface, const char *name, unsigned int (*send)(unsigned int offset, unsigned int count, void *buffer))
{

    resource_init(&interface->resource, 0, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, name);
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->in, SYSTEM_NODETYPE_MAILBOX, "in");
    system_initnode(&interface->out, SYSTEM_NODETYPE_NORMAL, "out");

    interface->send = send;
    interface->ctrl.resource = &interface->resource;
    interface->ctrl.read = interfacenode_ctrlread;
    interface->ctrl.write = interfacenode_ctrlwrite;
    interface->in.resource = &interface->resource;
    interface->out.resource = &interface->resource;
    interface->out.write = interfacenode_outwrite;

}

void module_init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "console");

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

