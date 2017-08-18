#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "pipe.h"

static struct system_node root;
static struct system_node clone;

static unsigned int end_open(struct system_node *self, struct service_state *state)
{

    struct pipe_end *end = (struct pipe_end *)self;

    end->refcount++;

    return state->id;

}

static unsigned int end_close(struct system_node *self, struct service_state *state)
{

    struct pipe_end *end = (struct pipe_end *)self;

    end->refcount--;

    if (end->read)
    {

        kernel_unblocktask(end->read->task);

        end->read = 0;

    }

    if (end->write)
    {

        kernel_unblocktask(end->write->task);

        end->write = 0;

    }

    return state->id;

}

static unsigned int end_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct pipe_end *end = (struct pipe_end *)self;

    count = ring_read(&end->ring, buffer, count);

    if (count)
    {

        if (end->write)
        {

            kernel_unblocktask(end->write->task);

            end->write = 0;

        }

    }

    if (!count)
    {

        if (end->refcount > 1)
        {

            end->read = state;

            kernel_blocktask(end->read->task);

        }

    }

    return count;

}

static unsigned int end_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct pipe_end *end = (struct pipe_end *)self;

    count = ring_write(&end->ring, buffer, count);

    if (count)
    {

        if (end->read)
        {

            kernel_unblocktask(end->read->task);

            end->read = 0;

        }

    }

    if (!count)
    {

        if (end->refcount > 1)
        {

            end->write = state;

            kernel_blocktask(end->write->task);

        }

    }

    return count;

}

static unsigned int clone_child(struct system_node *self, struct service_state *state, char *path, unsigned int length)
{

    struct list_item *current;

    for (current = root.children.head; current; current = current->next)
    {

        struct system_node *node = current->data;
        struct pipe *pipe = current->data;

        if (node == self)
            continue;

        if (pipe->end0.refcount || pipe->end1.refcount)
            continue;

        return node->child(node, state, path, length);

    }

    return 0;

}

void pipe_init(struct pipe *pipe, unsigned int count0, void *data0, unsigned int count1, void *data1)
{

    ring_init(&pipe->end0.ring, count0, data0);
    ring_init(&pipe->end1.ring, count1, data1);
    system_initnode(&pipe->end0.node, SYSTEM_NODETYPE_NORMAL, "0");
    system_initnode(&pipe->end1.node, SYSTEM_NODETYPE_NORMAL, "1");

    pipe->end0.node.open = end_open;
    pipe->end0.node.close = end_close;
    pipe->end0.node.read = end_read;
    pipe->end0.node.write = end_write;
    pipe->end1.node.open = end_open;
    pipe->end1.node.close = end_close;
    pipe->end1.node.read = end_read;
    pipe->end1.node.write = end_write;

    system_initnode(&pipe->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "pipe");
    system_addchild(&pipe->root, &pipe->end0.node);
    system_addchild(&pipe->root, &pipe->end1.node);

}

void pipe_register(struct pipe *pipe)
{

    system_addchild(&root, &pipe->root);

}

void pipe_unregister(struct pipe *pipe)
{

    system_removechild(&root, &pipe->root);

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "pipe");
    system_initnode(&clone, SYSTEM_NODETYPE_GROUP, "clone");

    clone.child = clone_child;

    system_addchild(&root, &clone);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

