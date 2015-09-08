#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "console.h"

void console_notify(struct console_interface *interface, unsigned int count, void *buffer)
{

    interface->in.write(&interface->in, 0, count, buffer);

}

static unsigned int interfacectrl_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interface *interface = self->resource->data;
    struct ctrl_consolesettings *settings = buffer;

    return memory_read(settings, count, &interface->settings, sizeof (struct ctrl_consolesettings), offset);

}

static unsigned int interfacectrl_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int interfaceout_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interface *interface = self->resource->data;

    return interface->send(count, buffer);

}

void console_registerinterface(struct console_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->in);
    system_addchild(&interface->root, &interface->out);
    system_registernode(&interface->root);

    interface->id = id;

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->in);
    system_removechild(&interface->root, &interface->out);
    system_unregisternode(&interface->root);

}

void console_initinterface(struct console_interface *interface, unsigned int (*send)(unsigned int count, void *buffer))
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "console");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->in, SYSTEM_NODETYPE_MAILBOX, "in");
    system_initnode(&interface->out, SYSTEM_NODETYPE_NORMAL, "out");

    interface->send = send;
    interface->ctrl.resource = &interface->resource;
    interface->ctrl.read = interfacectrl_read;
    interface->ctrl.write = interfacectrl_write;
    interface->in.resource = &interface->resource;
    interface->out.resource = &interface->resource;
    interface->out.write = interfaceout_write;

}

