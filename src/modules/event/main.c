#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "event.h"

static struct system_node root;
static struct system_node poll;

static unsigned int notify(void *buffer)
{

    struct event_header *header = buffer;
    unsigned int count = sizeof (struct event_header) + header->count;

    if (!poll.links.count)
        return 0;

    if (!header->destination)
        header->destination = (unsigned int)poll.links.head->data;

    return system_send(header->destination, count, buffer);

}

void event_notifykeypress(unsigned char scancode)
{

    struct {struct event_header header; struct event_keypress keypress;} message;

    message.header.type = EVENT_KEYPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_keypress);
    message.keypress.scancode = scancode;

    notify(&message);

}

void event_notifykeyrelease(unsigned char scancode)
{

    struct {struct event_header header; struct event_keyrelease keyrelease;} message;

    message.header.type = EVENT_KEYRELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_keyrelease);
    message.keyrelease.scancode = scancode;

    notify(&message);

}

void event_notifymousemove(char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.header.type = EVENT_MOUSEMOVE;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_mousemove);
    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    notify(&message);

}

void event_notifymousepress(unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.header.type = EVENT_MOUSEPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_mousepress);
    message.mousepress.button = button;

    notify(&message);

}

void event_notifymouserelease(unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.header.type = EVENT_MOUSERELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_mouserelease);
    message.mouserelease.button = button;

    notify(&message);

}

void event_notifytick(unsigned int counter)
{

    struct {struct event_header header; struct event_tick tick;} message;

    message.header.type = EVENT_TICK;
    message.header.source = 0;
    message.header.destination = 0;
    message.header.count = sizeof (struct event_tick);
    message.tick.counter = counter;

    notify(&message);

}

static unsigned int poll_write(struct system_node *self, struct list_item *link, struct service_state *state, unsigned int count, void *buffer)
{

    struct event_header *header = buffer;

    header->source = (unsigned int)link->data;

    return notify(buffer);

}

void module_init(void)
{

    system_initnode(&poll, SYSTEM_NODETYPE_MAILBOX, "poll");

    poll.write = poll_write;

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "event");
    system_addchild(&root, &poll);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

