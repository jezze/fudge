#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>
#include "event.h"

static struct system_node root;
static struct system_node send;
static struct system_node wm;
static struct list mailboxes;

void event_notify(unsigned int type, unsigned int count, void *buffer)
{

    struct list_item *current;
    struct event_header header;

    header.destination = 0xFFFFFFFF;
    header.source = 0;
    header.type = type;
    header.count = count;

    for (current = mailboxes.head; current; current = current->next)
    {

        struct task_mailbox *mailbox = current->data;

        scheduler_mailbox_write(mailbox, sizeof (struct event_header), &header);
        scheduler_mailbox_write(mailbox, count, buffer);

    }

}

static unsigned int send_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct list_item *current;

/*
    struct event_header *header = buffer;

    if (!header->destination)
        return 0;
*/

    for (current = mailboxes.head; current; current = current->next)
    {

        struct task_mailbox *mailbox = current->data;

        scheduler_mailbox_write(mailbox, count, buffer);

    }

    return count;

}

static unsigned int wm_open(struct system_node *self)
{

    scheduler_activetask_addmailbox(&mailboxes);

    return system_open(self);

}

static unsigned int wm_close(struct system_node *self)
{

    scheduler_activetask_removemailbox(&mailboxes);

    return system_close(self);

}

static unsigned int wm_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return scheduler_activetask_readmailbox(count, buffer);

}

void init()
{

    list_init(&mailboxes);
    system_initnode(&send, SYSTEM_NODETYPE_NORMAL, "send");

    send.write = send_write;

    system_initnode(&wm, SYSTEM_NODETYPE_NORMAL, "wm");

    wm.open = wm_open;
    wm.close = wm_close;
    wm.read = wm_read;

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "event");
    system_addchild(&root, &send);
    system_addchild(&root, &wm);
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

