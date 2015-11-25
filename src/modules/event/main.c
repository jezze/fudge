#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "event.h"

static struct system_node root;
static struct system_node poll;

void event_notify(unsigned int type, unsigned int count, void *buffer)
{

    struct event_header header;
    struct task *destination;

    if (!poll.mailboxes.count)
        return;

    header.destination = (unsigned int)poll.mailboxes.head->data;
    header.source = 0;
    header.type = type;
    header.count = count;

    destination = (struct task *)header.destination;

    task_setstatus(destination, TASK_STATUS_ACTIVE);

    buffer_wcfifo(&destination->mailbox.buffer, sizeof (struct event_header), &header);
    buffer_wcfifo(&destination->mailbox.buffer, count, buffer);

}

static unsigned int poll_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct event_header *header = buffer;
    struct task *destination;

    header->source = (unsigned int)task_findactive();

    if (!header->destination)
        header->destination = (unsigned int)poll.mailboxes.head->data;

    destination = (struct task *)header->destination;

    task_setstatus(destination, TASK_STATUS_ACTIVE);

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

