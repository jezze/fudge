#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "pipe.h"

static struct system_node root;
static struct system_node clone;

static unsigned int p0_open(struct system_node *self)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    pipe->t0 = task_findactive();

    return (unsigned int)self;

}

static unsigned int p0_close(struct system_node *self)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    pipe->t0 = 0;

    if (pipe->t1)
        task_setstatus(pipe->t1, TASK_STATUS_ACTIVE);

    return (unsigned int)self;

}

static unsigned int p0_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    if (pipe->t1)
    {

        task_setstatus(pipe->t1, TASK_STATUS_ACTIVE);

        return task_rmessage(pipe->t0, count, buffer);

    }

    else
    {

        return 0;

    }

}

static unsigned int p0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    if (pipe->t1)
    {

        return task_wmessage(pipe->t1, count, buffer);

    }

    else
    {

        task_setstatus(pipe->t0, TASK_STATUS_BLOCKED);

        return 0;

    }

}

static unsigned int p1_open(struct system_node *self)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    pipe->t1 = task_findactive();

    return (unsigned int)self;

}

static unsigned int p1_close(struct system_node *self)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    pipe->t1 = 0;

    if (pipe->t0)
        task_setstatus(pipe->t0, TASK_STATUS_ACTIVE);

    return (unsigned int)self;

}

static unsigned int p1_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    if (pipe->t0)
    {

        task_setstatus(pipe->t0, TASK_STATUS_ACTIVE);

        return task_rmessage(pipe->t1, count, buffer);

    }

    else
    {

        return 0;

    }

}

static unsigned int p1_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    if (pipe->t0)
    {

        return task_wmessage(pipe->t0, count, buffer);

    }

    else
    {

        task_setstatus(pipe->t1, TASK_STATUS_BLOCKED);

        return 0;

    }

}

static unsigned int clone_child(struct system_node *self, unsigned int count, char *path)
{

    struct list_item *current;

    for (current = root.children.head; current; current = current->next)
    {

        struct system_node *node = current->data;
        struct pipe *pipe = current->data;

        if (node == self)
            continue;

        if (pipe->t0 || pipe->t1)
            continue;

        return node->child(node, count, path);

    }

    return 0;

}

void pipe_init(struct pipe *pipe)
{

    pipe->t0 = 0;
    pipe->t1 = 0;

    system_initnode(&pipe->p0, SYSTEM_NODETYPE_NORMAL, "0");
    system_initnode(&pipe->p1, SYSTEM_NODETYPE_NORMAL, "1");

    pipe->p0.open = p0_open;
    pipe->p0.close = p0_close;
    pipe->p0.read = p0_read;
    pipe->p0.write = p0_write;
    pipe->p1.open = p1_open;
    pipe->p1.close = p1_close;
    pipe->p1.read = p1_read;
    pipe->p1.write = p1_write;

    system_initnode(&pipe->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "pipe");
    system_addchild(&pipe->root, &pipe->p0);
    system_addchild(&pipe->root, &pipe->p1);

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

