#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "event.h"

static struct system_node root;
static struct system_node poll;

static void event_notify(void *buffer)
{

    struct list_item *current;
    struct event_header *header = buffer;
    unsigned int count = sizeof (struct event_header) + header->count;

    for (current = poll.links.head; current; current = current->next)
    {

        struct task *task = current->data;

        header->destination = (unsigned int)task;

        if (task->mailbox.buffer.capacity - task->mailbox.buffer.count < count)
            break;

        task_setstatus(task, TASK_STATUS_UNBLOCKED);
        buffer_wcfifo(&task->mailbox.buffer, count, buffer);

        break;

    }

}

void event_notifykeypress(unsigned char scancode)
{

    struct {struct event_header header; struct event_keypress keypress;} message;

    message.header.type = EVENT_KEYPRESS;
    message.header.source = 0;
    message.header.count = sizeof (struct event_keypress);
    message.keypress.scancode = scancode;

    event_notify(&message);

}

void event_notifykeyrelease(unsigned char scancode)
{

    struct {struct event_header header; struct event_keyrelease keyrelease;} message;

    message.header.type = EVENT_KEYRELEASE;
    message.header.source = 0;
    message.header.count = sizeof (struct event_keyrelease);
    message.keyrelease.scancode = scancode;

    event_notify(&message);

}

void event_notifymousemove(char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.header.type = EVENT_MOUSEMOVE;
    message.header.source = 0;
    message.header.count = sizeof (struct event_mousemove);
    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    event_notify(&message);

}

void event_notifymousepress(unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.header.type = EVENT_MOUSEPRESS;
    message.header.source = 0;
    message.header.count = sizeof (struct event_mousepress);
    message.mousepress.button = button;

    event_notify(&message);

}

void event_notifymouserelease(unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.header.type = EVENT_MOUSERELEASE;
    message.header.source = 0;
    message.header.count = sizeof (struct event_mouserelease);
    message.mouserelease.button = button;

    event_notify(&message);

}

void event_notifytick(unsigned int counter)
{

    struct {struct event_header header; struct event_tick tick;} message;

    message.header.type = EVENT_TICK;
    message.header.source = 0;
    message.header.count = sizeof (struct event_tick);
    message.tick.counter = counter;

    event_notify(&message);

}

static unsigned int poll_write(struct system_node *self, struct task *task, unsigned int descriptor, unsigned int offset, unsigned int count, void *buffer)
{

    struct event_header *header = buffer;
    struct task *destination;

    header->source = (unsigned int)task;

    if (!header->destination)
        header->destination = (unsigned int)poll.links.head->data;

    destination = (struct task *)header->destination;

    task_setstatus(destination, TASK_STATUS_UNBLOCKED);

    return buffer_wcfifo(&destination->mailbox.buffer, count, buffer);

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

