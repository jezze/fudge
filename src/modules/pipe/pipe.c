#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "pipe.h"

static struct pipe pipe;

static struct system_node *pipe_open(struct system_node *self, struct service_state *state)
{

    list_add(&self->states, &state->item);
    pipe_use(&pipe, state);

    return self;

}

static struct system_node *pipe_close(struct system_node *self, struct service_state *state)
{

    list_remove(&self->states, &state->item);

    if (pipe.idata.states.count || pipe.odata.states.count)
        kernel_blocktask(state->task);
    else
        pipe_free(&pipe, state);

    return self;

}

static struct system_node *idata_open(struct system_node *self, struct service_state *state)
{

    list_add(&pipe.idata.states, &state->item);

    if (!pipe.odata.states.count)
        kernel_blocktask(state->task);

    pipe_unblock(&pipe.odata.states);

    return self;

}

static struct system_node *idata_close(struct system_node *self, struct service_state *state)
{

    list_remove(&pipe.idata.states, &state->item);
    pipe_unblock(&pipe.odata.states);
    pipe_unblock(&pipe.root.states);

    return self;

}

static unsigned int idata_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    spinlock_acquire(&pipe.datalock);

    count = ring_read(&pipe.ring, buffer, count);

    spinlock_release(&pipe.datalock);

    if (!count && pipe.odata.states.count)
        kernel_blocktask(state->task);

    pipe_unblock(&pipe.odata.states);

    return count;

}

static struct system_node *odata_open(struct system_node *self, struct service_state *state)
{

    list_add(&pipe.odata.states, &state->item);

    if (!pipe.idata.states.count)
        kernel_blocktask(state->task);

    pipe_unblock(&pipe.idata.states);

    return self;

}

static struct system_node *odata_close(struct system_node *self, struct service_state *state)
{

    list_remove(&pipe.odata.states, &state->item);
    pipe_unblock(&pipe.idata.states);
    pipe_unblock(&pipe.root.states);

    return self;

}

static unsigned int odata_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    spinlock_acquire(&pipe.datalock);

    count = ring_write(&pipe.ring, buffer, count);

    spinlock_release(&pipe.datalock);

    if (!count && pipe.idata.states.count)
        kernel_blocktask(state->task);

    pipe_unblock(&pipe.idata.states);

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

