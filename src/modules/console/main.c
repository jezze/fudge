#include <fudge.h>
#include <kernel.h>
#include <event/base.h>
#include <event/device.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "console.h"

static struct system_node root;
static struct system_node event;

void console_notify(struct console_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicast(&interface->idata.states, buffer, count);

}

void console_notifydata(struct console_interface *interface, unsigned char data)
{

    struct {struct event_header header; struct event_consoledata consoledata;} message;

    message.header.type = EVENT_CONSOLEDATA;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.header.length = sizeof (message);
    message.consoledata.data = data;

    event_multicast(&event.states, &message.header);
    event_multicast(&interface->event.states, &message.header);

}

void console_registerinterface(struct console_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->idata);
    system_addchild(&interface->root, &interface->odata);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->idata);
    system_removechild(&interface->root, &interface->odata);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void console_initinterface(struct console_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->idata, SYSTEM_NODETYPE_MAILBOX, "idata");
    system_initnode(&interface->odata, SYSTEM_NODETYPE_NORMAL, "odata");
    system_initnode(&interface->event, SYSTEM_NODETYPE_MAILBOX, "event");

    interface->id = id;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "console");
    system_initnode(&event, SYSTEM_NODETYPE_MAILBOX, "event");

}

void module_register(void)
{

    system_registernode(&root);
    system_addchild(&root, &event);

}

void module_unregister(void)
{

    system_unregisternode(&root);
    system_removechild(&root, &event);

}

