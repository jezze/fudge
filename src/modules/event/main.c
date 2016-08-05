#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "event.h"

static struct system_node root;
static struct system_node poll;
static struct system_node keypress;
static struct system_node keyrelease;
static struct system_node mousepress;
static struct system_node mouserelease;
static struct system_node mousemove;
static struct system_node wm;
static struct system_node tick;

static void unicast(struct list *list, struct event_header *header, unsigned int count)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->data;

        if (header->destination != (unsigned int)task)
            continue;

        task_setstatus(task, TASK_STATUS_UNBLOCKED);
        buffer_wcfifo(&task->mailbox.buffer, count, header);

    }

}

static void multicast(struct list *list, struct event_header *header, unsigned int count)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->data;

        header->destination = (unsigned int)task;

        task_setstatus(task, TASK_STATUS_UNBLOCKED);
        buffer_wcfifo(&task->mailbox.buffer, count, header);

    }

}

void event_notifykeypress(unsigned char scancode)
{

    struct {struct event_header header; struct event_keypress keypress;} message;

    message.header.type = EVENT_KEYPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.keypress.scancode = scancode;

    multicast(&keypress.links, &message.header, sizeof (struct event_header) + sizeof (struct event_keypress));

}

void event_notifykeyrelease(unsigned char scancode)
{

    struct {struct event_header header; struct event_keyrelease keyrelease;} message;

    message.header.type = EVENT_KEYRELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.keyrelease.scancode = scancode;

    multicast(&keyrelease.links, &message.header, sizeof (struct event_header) + sizeof (struct event_keyrelease));

}

void event_notifymousemove(char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.header.type = EVENT_MOUSEMOVE;
    message.header.source = 0;
    message.header.destination = 0;
    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    multicast(&mousemove.links, &message.header, sizeof (struct event_header) + sizeof (struct event_mousemove));

}

void event_notifymousepress(unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.header.type = EVENT_MOUSEPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.mousepress.button = button;

    multicast(&mousepress.links, &message.header, sizeof (struct event_header) + sizeof (struct event_mousepress));

}

void event_notifymouserelease(unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.header.type = EVENT_MOUSERELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.mouserelease.button = button;

    multicast(&mouserelease.links, &message.header, sizeof (struct event_header) + sizeof (struct event_mouserelease));

}

void event_notifytick(unsigned int counter)
{

    struct {struct event_header header; struct event_tick tick;} message;

    message.header.type = EVENT_TICK;
    message.header.source = 0;
    message.header.destination = 0;
    message.tick.counter = counter;

    multicast(&tick.links, &message.header, sizeof (struct event_header) + sizeof (struct event_tick));

}

static unsigned int write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct event_header *header = buffer;

    if (!header->source)
        header->source = (unsigned int)state->link.data;

    if (header->destination)
        unicast(&self->links, header, count);
    else
        multicast(&self->links, header, count);

    return count;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "event");
    system_initnode(&poll, SYSTEM_NODETYPE_MAILBOX, "poll");
    system_initnode(&keypress, SYSTEM_NODETYPE_MAILBOX, "keypress");
    system_initnode(&keyrelease, SYSTEM_NODETYPE_MAILBOX, "keyrelease");
    system_initnode(&mousepress, SYSTEM_NODETYPE_MAILBOX, "mousepress");
    system_initnode(&mouserelease, SYSTEM_NODETYPE_MAILBOX, "mouserelease");
    system_initnode(&mousemove, SYSTEM_NODETYPE_MAILBOX, "mousemove");
    system_initnode(&wm, SYSTEM_NODETYPE_MAILBOX, "wm");
    system_initnode(&tick, SYSTEM_NODETYPE_MAILBOX, "tick");

    poll.write = write;
    keypress.write = write;
    keyrelease.write = write;
    mousepress.write = write;
    mouserelease.write = write;
    mousemove.write = write;
    wm.write = write;
    tick.write = write;

    system_addchild(&root, &poll);
    system_addchild(&root, &keypress);
    system_addchild(&root, &keyrelease);
    system_addchild(&root, &mousepress);
    system_addchild(&root, &mouserelease);
    system_addchild(&root, &mousemove);
    system_addchild(&root, &wm);
    system_addchild(&root, &tick);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

