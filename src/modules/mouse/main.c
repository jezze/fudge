#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "mouse.h"

static struct system_node root;
static struct system_node event;

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely)
{

    struct event_mousemove mousemove;

    mousemove.relx = relx;
    mousemove.rely = rely;

    kernel_notify(&event.states, EVENT_MOUSEMOVE, &mousemove, sizeof (struct event_mousemove));
    kernel_notify(&interface->event.states, EVENT_MOUSEMOVE, &mousemove, sizeof (struct event_mousemove));

}

void mouse_notifypress(struct mouse_interface *interface, unsigned int button)
{

    struct event_mousepress mousepress;

    mousepress.button = button;

    kernel_notify(&event.states, EVENT_MOUSEPRESS, &mousepress, sizeof (struct event_mousepress));
    kernel_notify(&interface->event.states, EVENT_MOUSEPRESS, &mousepress, sizeof (struct event_mousepress));

}

void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button)
{

    struct event_mouserelease mouserelease;

    mouserelease.button = button;

    kernel_notify(&event.states, EVENT_MOUSERELEASE, &mouserelease, sizeof (struct event_mouserelease));
    kernel_notify(&interface->event.states, EVENT_MOUSERELEASE, &mouserelease, sizeof (struct event_mouserelease));

}

void mouse_registerinterface(struct mouse_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void mouse_initinterface(struct mouse_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_MOUSEINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->event, SYSTEM_NODETYPE_NORMAL, "event");

    interface->id = id;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "mouse");
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

