#include <fudge.h>
#include <kernel.h>
#include <event/base.h>
#include <modules/system/system.h>
#include <modules/event/event.h>

static struct system_node root;
static struct system_node event;

static unsigned int event_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct event_header *header = buffer;

    if (count < sizeof (struct event_header))
        return 0;

    if (header->length != count)
        return 0;

    header->source = state->task->id;

    if (header->destination)
        return event_unicast(&self->states, header);
    else
        return event_multicast(&self->states, header);

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "wm");
    system_initnode(&event, SYSTEM_NODETYPE_MAILBOX, "event");

    event.operations.write = event_write;

}

void module_register(void)
{

    system_registernode(&root);
    system_addchild(&root, &event);

}

void module_unregister(void)
{

    system_unregisternode(&root);
    system_removechild(&root, &event);

}

