#include <fudge.h>
#include <kernel.h>
#include <event/base.h>
#include <modules/system/system.h>
#include "event.h"

static struct system_node root;

unsigned int event_unicast(struct list *states, struct event_header *header)
{

    struct list_item *current;

    spinlock_acquire(&states->spinlock);

    for (current = states->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        if (header->destination != state->task->id)
            continue;

        task_writeall(state->task, header, header->length);
        kernel_unblocktask(state->task);

    }

    spinlock_release(&states->spinlock);

    return header->length;

}

unsigned int event_multicast(struct list *states, struct event_header *header)
{

    struct list_item *current;

    spinlock_acquire(&states->spinlock);

    for (current = states->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        header->destination = state->task->id;

        task_writeall(state->task, header, header->length);
        kernel_unblocktask(state->task);

    }

    spinlock_release(&states->spinlock);

    return header->length;

}

static unsigned int root_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    count = task_read(state->task, buffer, count);

    if (!count)
        kernel_blocktask(state->task);

    return count;

}

static unsigned int root_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct event_header *header = buffer;

    if (count < sizeof (struct event_header))
        return 0;

    if (header->length != count)
        return 0;

    if (!header->source)
        header->source = state->task->id;

    if (header->destination)
        return event_unicast(&self->states, header);
    else
        return event_multicast(&self->states, header);

}

static unsigned int root_seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return 0;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_MAILBOX, "event");

    root.operations.read = root_read;
    root.operations.write = root_write;
    root.operations.seek = root_seek;

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

