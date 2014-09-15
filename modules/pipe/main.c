#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>

struct pipe_endpoint
{

    struct system_node pipe;
    unsigned char buffer[4096];
    struct buffer cfifo;
    struct scheduler_rendezvous rdata;
    struct scheduler_rendezvous wdata;

};

static struct pipe_endpoint pipe0;
static struct pipe_endpoint pipe1;
static struct system_node root;

static unsigned int pipe0_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&pipe1.cfifo, count, buffer);

    scheduler_rendezvous_sleep(&pipe0.rdata, !count && pipe1.pipe.refcount);
    scheduler_rendezvous_unsleep(&pipe1.wdata);

    return count;

}

static unsigned int pipe0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int c = buffer_wcfifo(&pipe0.cfifo, count, buffer);

    scheduler_rendezvous_sleep(&pipe0.wdata, !c && count);
    scheduler_rendezvous_unsleep(&pipe1.rdata);

    return c;

}

static unsigned int pipe1_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&pipe0.cfifo, count, buffer);

    scheduler_rendezvous_sleep(&pipe1.rdata, !count && pipe0.pipe.refcount);
    scheduler_rendezvous_unsleep(&pipe0.wdata);

    return count;

}

static unsigned int pipe1_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int c = buffer_wcfifo(&pipe1.cfifo, count, buffer);

    scheduler_rendezvous_sleep(&pipe1.wdata, !c && count);
    scheduler_rendezvous_unsleep(&pipe0.rdata);

    return c;

}

void init()
{

    memory_clear(&pipe0, sizeof (struct pipe_endpoint));
    buffer_init(&pipe0.cfifo, 1, 4096, pipe0.buffer);
    system_initnode(&pipe0.pipe, SYSTEM_NODETYPE_NORMAL, "0");

    pipe0.pipe.read = pipe0_read;
    pipe0.pipe.write = pipe0_write;

    memory_clear(&pipe1, sizeof (struct pipe_endpoint));
    buffer_init(&pipe1.cfifo, 1, 4096, pipe1.buffer);
    system_initnode(&pipe1.pipe, SYSTEM_NODETYPE_NORMAL, "1");

    pipe1.pipe.read = pipe1_read;
    pipe1.pipe.write = pipe1_write;

    system_initnode(&root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "pipe");
    system_addchild(&root, &pipe0.pipe);
    system_addchild(&root, &pipe1.pipe);
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

