#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <event/event.h>
#include "console.h"

void console_notify(struct console_interface *interface, unsigned int count, void *buffer)
{

    scheduler_sendlist(&interface->in.mailboxes, count, buffer);
    event_notify(EVENT_CONSOLE, count, buffer);

}

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct console_interface *interface = self->resource->data;
    struct ctrl_consolesettings settings;

    settings.header.type = CTRL_TYPE_CONSOLE;
    settings.scroll = interface->scroll;

    return memory_read(buffer, count, &settings, sizeof (struct ctrl_consolesettings), size, offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int interfacenode_outwrite(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct console_interface *interface = self->resource->data;

    return interface->send(offset, size * count, buffer);

}

void console_registerinterface(struct console_interface *interface, unsigned int id)
{

    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->in);
    system_addchild(&interface->root, &interface->out);
    system_registernode(&interface->root);

    interface->id = id;

}

void console_unregisterinterface(struct console_interface *interface)
{

    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->in);
    system_removechild(&interface->root, &interface->out);
    system_unregisternode(&interface->root);

}

void console_initinterface(struct console_interface *interface, unsigned int (*send)(unsigned int offset, unsigned int count, void *buffer))
{

    resource_init(&interface->resource, 0, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "console");
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

}

void module_register()
{

}

void module_unregister()
{

}

