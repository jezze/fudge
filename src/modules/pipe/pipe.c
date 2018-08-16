#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "pipe.h"

static struct pipe pipe;

static void block(struct list *states, struct service_state *state)
{

    spinlock_acquire(&states->spinlock);

    if (!states->count)
        kernel_blocktask(state->task);

    spinlock_release(&states->spinlock);

}

static void block2(struct list *states, struct service_state *state)
{

    spinlock_acquire(&states->spinlock);

    if (states->count)
        kernel_blocktask(state->task);

    spinlock_release(&states->spinlock);

}

static void unblock(struct list *states)
{

    struct list_item *current;

    spinlock_acquire(&states->spinlock);

    for (current = states->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        kernel_unblocktask(state->task);

    }

    spinlock_release(&states->spinlock);

}

static struct system_node *pipe_open(struct system_node *self, struct service_state *state)
{

    list_add(&self->states, &state->item);
    pipe_use(&pipe);

    return self;

}

static struct system_node *pipe_close(struct system_node *self, struct service_state *state)
{

    if (pipe.idata.states.count || pipe.odata.states.count)
    {

        kernel_blocktask(state->task);

    }

    else
    {

        list_remove(&self->states, &state->item);
        pipe_free(&pipe);

    }

    return self;

}

static struct system_node *idata_open(struct system_node *self, struct service_state *state)
{

    list_add(&pipe.idata.states, &state->item);
    block(&pipe.odata.states, state);
    unblock(&pipe.odata.states);

    return self;

}

static struct system_node *idata_close(struct system_node *self, struct service_state *state)
{

    list_remove(&pipe.idata.states, &state->item);
    unblock(&pipe.odata.states);
    unblock(&pipe.root.states);

    return self;

}

static unsigned int idata_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    spinlock_acquire(&pipe.datalock);

    count = ring_read(&pipe.ring, buffer, count);

    spinlock_release(&pipe.datalock);

    if (!count)
        block2(&pipe.odata.states, state);

    unblock(&pipe.odata.states);

    return count;

}

static struct system_node *odata_open(struct system_node *self, struct service_state *state)
{

    list_add(&pipe.odata.states, &state->item);
    block(&pipe.idata.states, state);
    unblock(&pipe.idata.states);

    return self;

}

static struct system_node *odata_close(struct system_node *self, struct service_state *state)
{

    list_remove(&pipe.odata.states, &state->item);
    unblock(&pipe.idata.states);
    unblock(&pipe.root.states);

    return self;

}

static unsigned int odata_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    spinlock_acquire(&pipe.datalock);

    count = ring_write(&pipe.ring, buffer, count);

    spinlock_release(&pipe.datalock);

    if (!count)
        block2(&pipe.idata.states, state);

    unblock(&pipe.idata.states);

    return count;

}

void module_init(void)
{

    pipe_init(&pipe);

    pipe.root.operations.open = pipe_open;
    pipe.root.operations.close = pipe_close;
    pipe.idata.operations.open = idata_open;
    pipe.idata.operations.close = idata_close;
    pipe.idata.operations.read = idata_read;
    pipe.odata.operations.open = odata_open;
    pipe.odata.operations.close = odata_close;
    pipe.odata.operations.write = odata_write;

}

void module_register(void)
{

    pipe_register(&pipe);

}

void module_unregister(void)
{

    pipe_unregister(&pipe);

}

