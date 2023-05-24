#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "console.h"

static struct system_node root;
static struct system_node event;

void console_notifydata(struct console_interface *interface, unsigned char data)
{

    struct event_consoledata consoledata;

    consoledata.data = data;

    kernel_notify(&event.links, EVENT_CONSOLEDATA, sizeof (struct event_consoledata), &consoledata);
    kernel_notify(&interface->event.links, EVENT_CONSOLEDATA, sizeof (struct event_consoledata), &consoledata);

}

void console_registerinterface(struct console_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void console_initinterface(struct console_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&interface->event, SYSTEM_NODETYPE_NORMAL, "event");

    interface->id = id;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "console");
    system_initnode(&event, SYSTEM_NODETYPE_NORMAL, "event");

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

