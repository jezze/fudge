#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/rendezvous.h>
#include <system/system.h>
#include <base/base.h>

struct pipe_stream
{

    char buffer[4096];
    unsigned int head;
    unsigned int tail;

};

struct pipe_endpoint
{

    struct system_stream pipe;
    struct pipe_stream stream;
    struct rendezvous rread;

};

static struct pipe_endpoint pipe0;
static struct pipe_endpoint pipe1;
static struct system_group root;

static unsigned int read_stream(struct pipe_stream *stream, unsigned int count, void *buffer)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int tail = (stream->tail + 1) % 4096;

        if (stream->head == stream->tail)
            break;

        b[i] = stream->buffer[stream->tail];
        stream->tail = tail;

    }

    return i;

}

static unsigned int write_stream(struct pipe_stream *stream, unsigned int count, void *buffer)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int head = (stream->head + 1) % 4096;

        if (head == stream->tail)
            break;

        stream->buffer[stream->head] = b[i];
        stream->head = head;

    }

    return i;

}

static unsigned int pipe0_close(struct system_node *self)
{

    rendezvous_unsleep(&pipe0.rread);

    return (unsigned int)self;

}

static unsigned int pipe0_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = read_stream(&pipe1.stream, count, buffer);

    rendezvous_sleep(&pipe1.rread, !count && rendezvous_asleep(&pipe0.rread));

    return count;

}

static unsigned int pipe0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = write_stream(&pipe0.stream, count, buffer);

    rendezvous_unsleep(&pipe0.rread);

    return count;

}

static unsigned int pipe1_close(struct system_node *self)
{

    rendezvous_unsleep(&pipe1.rread);

    return (unsigned int)self;

}

static unsigned int pipe1_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = read_stream(&pipe0.stream, count, buffer);

    rendezvous_sleep(&pipe0.rread, !count && rendezvous_asleep(&pipe1.rread));

    return count;

}

static unsigned int pipe1_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = write_stream(&pipe1.stream, count, buffer);

    rendezvous_unsleep(&pipe1.rread);

    return count;

}

void init()
{

    system_init_stream(&pipe0.pipe, "0");

    pipe0.pipe.node.close = pipe0_close;
    pipe0.pipe.node.read = pipe0_read;
    pipe0.pipe.node.write = pipe0_write;

    system_init_stream(&pipe1.pipe, "1");

    pipe1.pipe.node.close = pipe1_close;
    pipe1.pipe.node.read = pipe1_read;
    pipe1.pipe.node.write = pipe1_write;

    system_init_group(&root, "pipe");
    system_group_add(&root, &pipe0.pipe.node);
    system_group_add(&root, &pipe1.pipe.node);
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

