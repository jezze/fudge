#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "event.h"

static struct system_node root;
static struct system_node poll;
static struct system_node send;

void event_notify(unsigned int size, unsigned int count, void *buffer)
{

    struct event_header *header = buffer;

    header->destination = 0xFFFFFFFF;
    header->source = 0;

    system_write(&poll, 0, size, count, buffer);

}

static unsigned int send_write(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct event_header *header = buffer;
    struct task *destination;

    header->source = (unsigned int)scheduler_findactive();

    if (header->destination == 0xFFFFFFFF)
    {

        system_write(&poll, 0, size, count, buffer);

        return count;

    }

    destination = (struct task *)header->destination;

    return scheduler_wmessage(destination, size, count, buffer);

}

void module_init()
{

    system_initnode(&poll, SYSTEM_NODETYPE_MAILBOX, "poll");
    system_initnode(&send, SYSTEM_NODETYPE_NORMAL, "send");

    send.write = send_write;

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "event");
    system_addchild(&root, &poll);
    system_addchild(&root, &send);

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

