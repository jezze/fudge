#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>
#include "event.h"

struct event_endpoint
{

    struct system_node node;
    unsigned char buffer[4096];
    struct buffer cfifo;
    struct scheduler_rendezvous rdata;

};

static struct event_endpoint endpoint0;
static struct system_node root;

static unsigned int endpoint0_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&endpoint0.cfifo, count, buffer);

    if (!count)
        scheduler_rendezvous_sleep(&endpoint0.rdata);

    return count;

}

static unsigned int endpoint0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_wcfifo(&endpoint0.cfifo, count, buffer);

    if (count)
        scheduler_rendezvous_unsleep(&endpoint0.rdata);

    return count;

}

void event_notify(unsigned int type, unsigned int count, void *buffer)
{

    struct event_header header;

    header.type = type;
    header.count = count;

    buffer_wcfifo(&endpoint0.cfifo, sizeof (struct event_header), &header);
    buffer_wcfifo(&endpoint0.cfifo, count, buffer);

    scheduler_rendezvous_unsleep(&endpoint0.rdata);

}

void init()
{

    buffer_init(&endpoint0.cfifo, 1, 4096, endpoint0.buffer);
    system_initnode(&endpoint0.node, SYSTEM_NODETYPE_NORMAL, "0");

    endpoint0.node.read = endpoint0_read;
    endpoint0.node.write = endpoint0_write;

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "event");
    system_addchild(&root, &endpoint0.node);
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

