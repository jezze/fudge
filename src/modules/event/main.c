#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node eventnode;
static struct system_node multicastnode;

static unsigned int multicast(unsigned int source, struct list *states, struct event_header *header)
{

    struct list_item *current;

    spinlock_acquire(&states->spinlock);

    for (current = states->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        kernel_place(source, state->id, header);

    }

    spinlock_release(&states->spinlock);

    return header->length;

}

static unsigned int eventnode_seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return 0;

}

static unsigned int multicastnode_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    union event_message *message = buffer;

    if (count < sizeof (struct event_header))
        return 0;

    if (message->header.length != count)
        return 0;

    return multicast(state->id, &eventnode.states, &message->header);

}

static unsigned int multicastnode_seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return 0;

}

void module_init(void)
{

    system_initnode(&eventnode, SYSTEM_NODETYPE_NORMAL, "event");

    eventnode.operations.seek = eventnode_seek;

    system_initnode(&multicastnode, SYSTEM_NODETYPE_NORMAL, "multicast");

    multicastnode.operations.write = multicastnode_write;
    multicastnode.operations.seek = multicastnode_seek;

}

void module_register(void)
{

    system_registernode(&eventnode);
    system_registernode(&multicastnode);

}

void module_unregister(void)
{

    system_unregisternode(&eventnode);
    system_unregisternode(&multicastnode);

}

