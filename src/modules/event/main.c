#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "event.h"

static struct system_node root;
static struct system_node poll;

static void event_notify(unsigned int type, unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = poll.mailboxes.head; current; current = current->next)
    {

        struct event_header header;
        struct task *destination = current->data;

        header.destination = (unsigned int)destination;
        header.source = 0;
        header.type = type;
        header.count = count;

        task_setstatus(destination, TASK_STATUS_UNBLOCKED);
        buffer_wcfifo(&destination->mailbox.buffer, sizeof (struct event_header), &header);
        buffer_wcfifo(&destination->mailbox.buffer, count, buffer);

        break;

    }

}

void event_notifykeypress(unsigned char scancode)
{

    struct event_keypress keypress;

    keypress.scancode = scancode;

    event_notify(EVENT_KEYPRESS, sizeof (struct event_keypress), &keypress);

}

void event_notifykeyrelease(unsigned char scancode)
{

    struct event_keyrelease keyrelease;

    keyrelease.scancode = scancode;

    event_notify(EVENT_KEYRELEASE, sizeof (struct event_keyrelease), &keyrelease);

}

void event_notifymousemove(char relx, char rely)
{

    struct event_mousemove mousemove;

    mousemove.relx = relx;
    mousemove.rely = rely;

    event_notify(EVENT_MOUSEMOVE, sizeof (struct event_mousemove), &mousemove);

}

void event_notifymousepress(unsigned int button)
{

    struct event_mousepress mousepress;

    mousepress.button = button;

    event_notify(EVENT_MOUSEPRESS, sizeof (struct event_mousepress), &mousepress);

}

void event_notifymouserelease(unsigned int button)
{

    struct event_mouserelease mouserelease;

    mouserelease.button = button;

    event_notify(EVENT_MOUSERELEASE, sizeof (struct event_mouserelease), &mouserelease);

}

void event_notifytick(unsigned int counter)
{

    struct event_tick tick;

    tick.counter = counter;

    event_notify(EVENT_TICK, sizeof (struct event_tick), &tick);

}

static unsigned int poll_open(struct system_node *self)
{

    struct task *task = task_findactive();

    list_add(&self->mailboxes, &task->blockitem);

    return (unsigned int)self;

}

static unsigned int poll_close(struct system_node *self)
{

    struct task *task = task_findactive();

    list_remove(&self->mailboxes, &task->blockitem);

    return (unsigned int)self;

}

static unsigned int poll_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct event_header *header = buffer;
    struct task *destination;

    header->source = (unsigned int)task_findactive();

    if (!header->destination)
        header->destination = (unsigned int)poll.mailboxes.head->data;

    destination = (struct task *)header->destination;

    task_setstatus(destination, TASK_STATUS_UNBLOCKED);

    return buffer_wcfifo(&destination->mailbox.buffer, count, buffer);

}

void module_init(void)
{

    system_initnode(&poll, SYSTEM_NODETYPE_MAILBOX, "poll");

    poll.open = poll_open;
    poll.close = poll_close;
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

