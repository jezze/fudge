#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "pipe.h"

struct pipe_stream
{

    char buffer[4096];
    unsigned int head;
    unsigned int tail;

};

struct pipe_group
{

    struct system_group base;
    struct system_stream control;
    struct system_stream ipipe;
    struct system_stream opipe;
    char name[8];
    struct pipe_stream istream;
    struct pipe_stream ostream;

};

static struct system_group root;
static struct system_stream clone;
static struct pipe_group groups[8];

unsigned int stream_read(struct pipe_stream *stream, unsigned int count, void *buffer)
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

unsigned int stream_write(struct pipe_stream *stream, unsigned int count, void *buffer)
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

static unsigned int control_close(struct system_node *self)
{

    struct pipe_group *group = (struct pipe_group *)self->parent;
    struct system_group *root = (struct system_group *)self->parent->parent;

    system_group_remove(root, &group->base.node);

    return 0;

}

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_group *group = (struct pipe_group *)self->parent;

    return memory_read(buffer, count, group->name, string_length(group->name), offset);

}

static unsigned int ipipe_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_group *group = (struct pipe_group *)self->parent;

    return stream_read(&group->ostream, count, buffer);

}

static unsigned int ipipe_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_group *group = (struct pipe_group *)self->parent;

    return stream_write(&group->istream, count, buffer);

}

static unsigned int opipe_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_group *group = (struct pipe_group *)self->parent;

    return stream_read(&group->istream, count, buffer);

}

static unsigned int opipe_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe_group *group = (struct pipe_group *)self->parent;

    return stream_write(&group->ostream, count, buffer);

}

static unsigned int find_group()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!groups[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_group(struct pipe_group *group, unsigned int id)
{

    memory_clear(group, sizeof (struct pipe_group));

    memory_write_number(group->name, 8, id, 10, 0);
    system_init_group(&group->base, group->name);
    system_init_stream(&group->control, "control");
    system_group_add(&group->base, &group->control.node);
    system_init_stream(&group->ipipe, "0");
    system_group_add(&group->base, &group->ipipe.node);
    system_init_stream(&group->opipe, "1");
    system_group_add(&group->base, &group->opipe.node);

    group->control.node.close = control_close;
    group->control.node.read = control_read;
    group->ipipe.node.read = ipipe_read;
    group->ipipe.node.write = ipipe_write;
    group->opipe.node.read = opipe_read;
    group->opipe.node.write = opipe_write;

}

static unsigned int clone_open(struct system_node *self)
{

    unsigned int index = find_group();

    if (!index)
        return 0;

    init_group(&groups[index], index);
    system_group_add(&root, &groups[index].base.node);

    return (unsigned int)&groups[index].control;

}

void init()
{

    system_init_group(&root, "pipe");
    system_register_node(&root.node);
    system_init_stream(&clone, "clone");
    system_group_add(&root, &clone.node);

    clone.node.open = clone_open;

}

void destroy()
{

    system_unregister_node(&root.node);

}

