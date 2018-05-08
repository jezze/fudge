#include <fudge.h>
#include <kernel.h>
#include <event/event.h>
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

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_MAILBOX, "event");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

