#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include "event.h"

static struct list mailboxes;
static struct system_node root;
static struct system_node send;
static struct system_node wm;

void event_notify(unsigned int type, unsigned int count, void *buffer)
{

/*
    struct event_header header;

    header.destination = 0xFFFFFFFF;
    header.source = 0;
    header.type = type;
    header.count = count;

    scheduler_mailboxes_send(&mailboxes, sizeof (struct event_header), &header);
    scheduler_mailboxes_send(&mailboxes, count, buffer);
*/

}

static unsigned int send_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

/*
    return scheduler_mailboxes_send(&mailboxes, count, buffer);
*/

    return 0;

}

static unsigned int wm_open(struct system_node *self)
{

    scheduler_mailboxes_addactive(&mailboxes);

    return system_open(self);

}

static unsigned int wm_close(struct system_node *self)
{

    scheduler_mailboxes_removeactive(&mailboxes);

    return system_close(self);

}

static unsigned int wm_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return scheduler_mailboxes_readactive(&mailboxes, count, buffer);

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

