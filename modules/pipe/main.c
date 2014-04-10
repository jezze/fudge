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

static struct pipe_session
{

    struct system_group base;
    struct system_stream ipipe;
    struct system_stream opipe;
    char name[8];
    struct pipe_stream istream;
    struct pipe_stream ostream;
    struct rendezvous iread;
    struct rendezvous oread;

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

static unsigned int ipipe_open(struct system_node *self)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    rendezvous_lock(&session->iread);

    return (unsigned int)self;

}

static unsigned int ipipe_close(struct system_node *self)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    rendezvous_unlock(&session->iread);

    return 0;

}

static unsigned int ipipe_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    count = read_stream(&session->ostream, count, buffer);

    rendezvous_sleep(&session->iread, 0);

    return count;

}

static unsigned int ipipe_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    count = write_stream(&session->istream, count, buffer);

    rendezvous_unsleep(&session->oread, 1);

    return count;

}

static unsigned int opipe_open(struct system_node *self)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    rendezvous_lock(&session->oread);

    return (unsigned int)self;

}

static unsigned int opipe_close(struct system_node *self)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    rendezvous_unlock(&session->oread);

    return 0;

}

static unsigned int opipe_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    count = read_stream(&session->istream, count, buffer);

    rendezvous_sleep(&session->oread, 0);

    return count;

}

static unsigned int opipe_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_session *session = (struct pipe_session *)self->parent;

    count = write_stream(&session->ostream, count, buffer);

    rendezvous_unsleep(&session->iread, 1);

    return count;

}

static void init_session(struct pipe_session *session, unsigned int id)
{

    memory_clear(session, sizeof (struct pipe_session));
    ascii_fromint(session->name, 8, id, 10);
    system_init_group(&session->base, session->name);
    system_init_stream(&session->ipipe, "0");
    system_init_stream(&session->opipe, "1");

    session->ipipe.node.open = ipipe_open;
    session->ipipe.node.close = ipipe_close;
    session->ipipe.node.read = ipipe_read;
    session->ipipe.node.write = ipipe_write;
    session->opipe.node.open = opipe_open;
    session->opipe.node.close = opipe_close;
    session->opipe.node.read = opipe_read;
    session->opipe.node.write = opipe_write;

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
        system_group_add(&sessions[i].base, &sessions[i].ipipe.node);
        system_group_add(&sessions[i].base, &sessions[i].opipe.node);

    }

}

void destroy()
{

    system_unregister_node(&root.node);

}

