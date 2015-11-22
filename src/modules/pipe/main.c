#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "pipe.h"

static struct system_node root;
static struct system_node clone;

static void wakeup(struct list *list)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->data;

        task_setstatus(task, TASK_STATUS_ACTIVE);

    }

}

static unsigned int read(struct buffer *b, struct list *mailboxes, unsigned int ref, unsigned int count, void *buffer)
{

    struct task *task = task_findactive();

    count = buffer_rcfifo(b, count, buffer);

    if (!count && ref)
        task_setstatus(task, TASK_STATUS_BLOCKED);

    wakeup(mailboxes);

    return count;

}

static unsigned int write(struct buffer *b, struct list *mailboxes, unsigned int ref, unsigned int count, void *buffer)
{

    struct task *task = task_findactive();

    count = buffer_wcfifo(b, count, buffer);

    if (!count)
        task_setstatus(task, TASK_STATUS_BLOCKED);

    wakeup(mailboxes);

    return count;

}

static unsigned int p0_open(struct system_node *self)
{

    struct pipe *pipe = (struct pipe *)self->parent;
    struct task *task = task_findactive();

    pipe->ref0++;

    if (!list_find(&self->mailboxes, &task->blockitem))
        list_add(&self->mailboxes, &task->blockitem);

    wakeup(&pipe->p1.mailboxes);

    return (unsigned int)self;

}

static unsigned int p0_close(struct system_node *self)
{

    struct pipe *pipe = (struct pipe *)self->parent;
    struct task *task = task_findactive();

    pipe->ref0--;

    if (list_find(&self->mailboxes, &task->blockitem))
        list_remove(&self->mailboxes, &task->blockitem);

    wakeup(&pipe->p1.mailboxes);

    return (unsigned int)self;

}

static unsigned int p0_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    return read(&pipe->buffer0, &pipe->p1.mailboxes, pipe->ref1, count, buffer);

}

static unsigned int p0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    return write(&pipe->buffer1, &pipe->p1.mailboxes, pipe->ref1, count, buffer);

}

static unsigned int p1_open(struct system_node *self)
{

    struct pipe *pipe = (struct pipe *)self->parent;
    struct task *task = task_findactive();

    pipe->ref1++;

    if (!list_find(&self->mailboxes, &task->blockitem))
        list_add(&self->mailboxes, &task->blockitem);

    wakeup(&pipe->p0.mailboxes);

    return (unsigned int)self;

}

static unsigned int p1_close(struct system_node *self)
{

    struct pipe *pipe = (struct pipe *)self->parent;
    struct task *task = task_findactive();

    pipe->ref1--;

    if (list_find(&self->mailboxes, &task->blockitem))
        list_remove(&self->mailboxes, &task->blockitem);

    wakeup(&pipe->p0.mailboxes);

    return (unsigned int)self;

}

static unsigned int p1_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    return read(&pipe->buffer1, &pipe->p0.mailboxes, pipe->ref0, count, buffer);

}

static unsigned int p1_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pipe *pipe = (struct pipe *)self->parent;

    return write(&pipe->buffer0, &pipe->p0.mailboxes, pipe->ref0, count, buffer);

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

        if (pipe->p0.mailboxes.count || pipe->p1.mailboxes.count)
            continue;

        return node->child(node, count, path);

    }

    return 0;

}

void pipe_init(struct pipe *pipe)
{

    buffer_init(&pipe->buffer0, 4096, pipe->data0);
    buffer_init(&pipe->buffer1, 4096, pipe->data1);
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

    pipe->ref0 = 0;
    pipe->ref1 = 0;

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

