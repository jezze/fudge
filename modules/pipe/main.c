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

    scheduler_rendezvous_sleep(&pipe0.rdata, !count && scheduler_rendezvous_asleep(&pipe1.wdata));
    scheduler_rendezvous_unsleep(&pipe1.wdata);

    return count;

}

static unsigned int pipe0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_wcfifo(&pipe0.cfifo, count, buffer);

    scheduler_rendezvous_sleep(&pipe0.wdata, !count);
    scheduler_rendezvous_unsleep(&pipe1.rdata);

    return count;

}

static unsigned int pipe1_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&pipe0.cfifo, count, buffer);

    scheduler_rendezvous_sleep(&pipe1.rdata, !count && scheduler_rendezvous_asleep(&pipe0.wdata));
    scheduler_rendezvous_unsleep(&pipe0.wdata);

    return count;

}

static unsigned int pipe1_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_wcfifo(&pipe1.cfifo, count, buffer);

    scheduler_rendezvous_sleep(&pipe1.wdata, !count);
    scheduler_rendezvous_unsleep(&pipe0.rdata);

    return count;

}

void init()
{

    memory_clear(&pipe0, sizeof (struct pipe_endpoint));
    buffer_init(&pipe0.cfifo, 4096, pipe0.buffer);
    system_initstream(&pipe0.pipe, "0");

    pipe0.pipe.read = pipe0_read;
    pipe0.pipe.write = pipe0_write;

    memory_clear(&pipe1, sizeof (struct pipe_endpoint));
    buffer_init(&pipe1.cfifo, 4096, pipe1.buffer);
    system_initstream(&pipe1.pipe, "1");

    pipe1.pipe.read = pipe1_read;
    pipe1.pipe.write = pipe1_write;

    system_initmultigroup(&root, "pipe");
    system_addchild(&root, &pipe0.pipe);
    system_addchild(&root, &pipe1.pipe);
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

