#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>

struct pipe_endpoint
{

    struct system_node node;
    unsigned char buffer[4096];
    struct buffer cfifo;
    struct scheduler_rendezvous rdata;

};

static struct pipe_endpoint endpoint0;
static struct pipe_endpoint endpoint1;
static struct system_node root;

static unsigned int endpoint0_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&endpoint0.cfifo, count, buffer);

    scheduler_rendezvous_unsleep(&endpoint0.rdata);
    scheduler_rendezvous_sleep(&endpoint0.rdata, !count && endpoint1.node.refcount);

    return count;

}

static unsigned int endpoint0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_wcfifo(&endpoint1.cfifo, count, buffer);

    scheduler_rendezvous_unsleep(&endpoint1.rdata);
    scheduler_rendezvous_sleep(&endpoint1.rdata, !count && endpoint0.node.refcount);

    return count;

}

static unsigned int endpoint1_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&endpoint1.cfifo, count, buffer);

    scheduler_rendezvous_unsleep(&endpoint1.rdata);
    scheduler_rendezvous_sleep(&endpoint1.rdata, !count && endpoint0.node.refcount);

    return count;

}

static unsigned int endpoint1_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_wcfifo(&endpoint0.cfifo, count, buffer);

    scheduler_rendezvous_unsleep(&endpoint0.rdata);
    scheduler_rendezvous_sleep(&endpoint0.rdata, !count && endpoint1.node.refcount);

    return count;

}

void init()
{

    memory_clear(&endpoint0, sizeof (struct pipe_endpoint));
    buffer_init(&endpoint0.cfifo, 1, 4096, endpoint0.buffer);
    system_initnode(&endpoint0.node, SYSTEM_NODETYPE_NORMAL, "0");

    endpoint0.node.read = endpoint0_read;
    endpoint0.node.write = endpoint0_write;

    memory_clear(&endpoint1, sizeof (struct pipe_endpoint));
    buffer_init(&endpoint1.cfifo, 1, 4096, endpoint1.buffer);
    system_initnode(&endpoint1.node, SYSTEM_NODETYPE_NORMAL, "1");

    endpoint1.node.read = endpoint1_read;
    endpoint1.node.write = endpoint1_write;

    system_initnode(&root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "pipe");
    system_addchild(&root, &endpoint0.node);
    system_addchild(&root, &endpoint1.node);
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

