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
    struct rendezvous rwrite;

};

static struct pipe_session
{

    struct system_group base;
    char name[8];
    struct pipe_endpoint pipe0;
    struct pipe_endpoint pipe1;

} sessions[8];

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

static unsigned int pipe0_open(struct system_node *self, unsigned int flags)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    if (rendezvous_lock(&session->pipe0.rread, flags & 0x01) || rendezvous_lock(&session->pipe0.rwrite, flags & 0x02))
        return (unsigned int)self;
    else
        return 0;

}

static unsigned int pipe0_close(struct system_node *self)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    rendezvous_unlock(&session->pipe0.rread, 1);
    rendezvous_unlock(&session->pipe0.rwrite, 1);
    rendezvous_unsleep(&session->pipe1.rread, 1);

    return (unsigned int)self;

}

static unsigned int pipe0_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    count = read_stream(&session->pipe1.stream, count, buffer);

    rendezvous_sleep(&session->pipe0.rread, !count && rendezvous_islocked(&session->pipe1.rwrite));

    return count;

}

static unsigned int pipe0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    count = write_stream(&session->pipe0.stream, count, buffer);

    rendezvous_unsleep(&session->pipe1.rread, count);

    return count;

}

static unsigned int pipe1_open(struct system_node *self, unsigned int flags)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    if (rendezvous_lock(&session->pipe1.rread, flags & 0x01) || rendezvous_lock(&session->pipe1.rwrite, flags & 0x02))
        return (unsigned int)self;
    else
        return 0;

}

static unsigned int pipe1_close(struct system_node *self)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    rendezvous_unlock(&session->pipe1.rread, 1);
    rendezvous_unlock(&session->pipe1.rwrite, 1);
    rendezvous_unsleep(&session->pipe0.rread, 1);

    return (unsigned int)self;

}

static unsigned int pipe1_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    count = read_stream(&session->pipe0.stream, count, buffer);

    rendezvous_sleep(&session->pipe1.rread, !count && rendezvous_islocked(&session->pipe0.rwrite));

    return count;

}

static unsigned int pipe1_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    count = write_stream(&session->pipe1.stream, count, buffer);

    rendezvous_unsleep(&session->pipe0.rread, count);

    return count;

}

static void init_session(struct pipe_session *session, unsigned int id)
{

    memory_clear(session, sizeof (struct pipe_session));
    ascii_fromint(session->name, 8, id, 10);
    system_init_group(&session->base, session->name);
    system_init_stream(&session->pipe0.pipe, "0");
    system_init_stream(&session->pipe1.pipe, "1");

    session->pipe0.pipe.node.open = pipe0_open;
    session->pipe0.pipe.node.close = pipe0_close;
    session->pipe0.pipe.node.read = pipe0_read;
    session->pipe0.pipe.node.write = pipe0_write;
    session->pipe1.pipe.node.open = pipe1_open;
    session->pipe1.pipe.node.close = pipe1_close;
    session->pipe1.pipe.node.read = pipe1_read;
    session->pipe1.pipe.node.write = pipe1_write;

}

void init()
{

    unsigned int i;

    memory_clear(sessions, sizeof (struct pipe_session) * 8);
    system_init_group(&root, "pipe");
    system_register_node(&root.node);

    for (i = 0; i < 8; i++)
    {

        init_session(&sessions[i], i);
        system_group_add(&root, &sessions[i].base.node);
        system_group_add(&sessions[i].base, &sessions[i].pipe0.pipe.node);
        system_group_add(&sessions[i].base, &sessions[i].pipe1.pipe.node);

    }

}

void destroy()
{

    system_unregister_node(&root.node);

}

