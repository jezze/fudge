#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>

struct pipe_endpoint
{

    struct system_stream pipe;
    unsigned char buffer[4096];
    struct buffer_cfifo cfifo;
    struct scheduler_rendezvous rdata;

};

static struct pipe_endpoint pipe0;
static struct pipe_endpoint pipe1;
static struct system_group root;

static unsigned int pipe0_close(struct system_node *self)
{

    scheduler_rendezvous_unsleep(&pipe0.rdata);

    return (unsigned int)self;

}

static unsigned int pipe0_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_read_cfifo(&pipe1.cfifo, count, buffer);

    scheduler_rendezvous_sleep(&pipe1.rdata, !count && scheduler_rendezvous_asleep(&pipe0.rdata));

    return count;

}

static unsigned int pipe0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_write_cfifo(&pipe0.cfifo, count, buffer);

    scheduler_rendezvous_unsleep(&pipe0.rdata);

    return count;

}

static unsigned int pipe1_close(struct system_node *self)
{

    scheduler_rendezvous_unsleep(&pipe1.rdata);

    return (unsigned int)self;

}

static unsigned int pipe1_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_read_cfifo(&pipe0.cfifo, count, buffer);

    scheduler_rendezvous_sleep(&pipe0.rdata, !count && scheduler_rendezvous_asleep(&pipe1.rdata));

    return count;

}

static unsigned int pipe1_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_write_cfifo(&pipe1.cfifo, count, buffer);

    scheduler_rendezvous_unsleep(&pipe1.rdata);

    return count;

}

void init()
{

    memory_clear(&pipe0, sizeof (struct pipe_endpoint));
    buffer_init_cfifo(&pipe0.cfifo, 4096, pipe0.buffer);
    system_init_stream(&pipe0.pipe, "0");

    pipe0.pipe.node.close = pipe0_close;
    pipe0.pipe.node.read = pipe0_read;
    pipe0.pipe.node.write = pipe0_write;

    memory_clear(&pipe1, sizeof (struct pipe_endpoint));
    buffer_init_cfifo(&pipe1.cfifo, 4096, pipe1.buffer);
    system_init_stream(&pipe1.pipe, "1");

    pipe1.pipe.node.close = pipe1_close;
    pipe1.pipe.node.read = pipe1_read;
    pipe1.pipe.node.write = pipe1_write;

    system_init_group(&root, "pipe");

    root.node.multi = 1;

    system_group_add(&root, &pipe0.pipe.node);
    system_group_add(&root, &pipe1.pipe.node);
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

