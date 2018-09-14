#include <fudge.h>
#include <kernel.h>
#include <event/base.h>
#include <event/device.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "keyboard.h"

static struct system_node root;
static struct system_node event;

void keyboard_notify(struct keyboard_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicast(&interface->data.states, buffer, count);

}

void keyboard_notifypress(struct keyboard_interface *interface, unsigned char scancode)
{

    struct {struct event_header header; struct event_keypress keypress;} message;

    message.header.type = EVENT_KEYPRESS;
    message.header.source = EVENT_ADDR_SELF;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.header.length = sizeof (message);
    message.keypress.scancode = scancode;

    event_multicast(&event.states, &message.header);
    event_multicast(&interface->event.states, &message.header);

}

void keyboard_notifyrelease(struct keyboard_interface *interface, unsigned char scancode)
{

    struct {struct event_header header; struct event_keyrelease keyrelease;} message;

    message.header.type = EVENT_KEYRELEASE;
    message.header.source = EVENT_ADDR_SELF;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.header.length = sizeof (message);
    message.keyrelease.scancode = scancode;

    event_multicast(&event.states, &message.header);
    event_multicast(&interface->event.states, &message.header);

}

void keyboard_registerinterface(struct keyboard_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void keyboard_initinterface(struct keyboard_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_KEYBOARDINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data");
    system_initnode(&interface->event, SYSTEM_NODETYPE_MAILBOX, "event");

    interface->id = id;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "keyboard");
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

