#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node endpoint0;
static struct system_node endpoint1;
static struct task *t0;
static struct task *t1;

static unsigned int endpoint0_open(struct system_node *self)
{

    t0 = task_findactive();
    self->refcount++;
    self->parent->refcount++;

    return (unsigned int)self;

}

static unsigned int endpoint0_close(struct system_node *self)
{

    if (t1)
        task_setstatus(t1, TASK_STATUS_ACTIVE);

    t0 = 0;
    self->refcount--;
    self->parent->refcount--;

    return (unsigned int)self;

}

static unsigned int endpoint0_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    if (t1)
        return task_rmessage(t0, count, buffer);
    else
        return 0;

}

static unsigned int endpoint0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    if (t1)
        return task_wmessage(t1, count, buffer);
    else
        return 0;

}

static unsigned int endpoint1_open(struct system_node *self)
{

    t1 = task_findactive();
    self->refcount++;
    self->parent->refcount++;

    return (unsigned int)self;

}

static unsigned int endpoint1_close(struct system_node *self)
{

    if (t0)
        task_setstatus(t0, TASK_STATUS_ACTIVE);

    t1 = 0;
    self->refcount--;
    self->parent->refcount--;

    return (unsigned int)self;

}

static unsigned int endpoint1_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    if (t0)
        return task_rmessage(t1, count, buffer);
    else
        return 0;

}

static unsigned int endpoint1_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    if (t0)
        return task_wmessage(t0, count, buffer);
    else
        return 0;

}

void module_init(void)
{

    t0 = 0;

    system_initnode(&endpoint0, SYSTEM_NODETYPE_NORMAL, "0");

    endpoint0.open = endpoint0_open;
    endpoint0.close = endpoint0_close;
    endpoint0.read = endpoint0_read;
    endpoint0.write = endpoint0_write;

    t1 = 0;

    system_initnode(&endpoint1, SYSTEM_NODETYPE_NORMAL, "1");

    endpoint1.open = endpoint1_open;
    endpoint1.close = endpoint1_close;
    endpoint1.read = endpoint1_read;
    endpoint1.write = endpoint1_write;

    system_initnode(&root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "pipe");
    system_addchild(&root, &endpoint0);
    system_addchild(&root, &endpoint1);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

