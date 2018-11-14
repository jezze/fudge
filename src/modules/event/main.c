#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;

static unsigned int root_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    union event_message *message = buffer;

    if (count < sizeof (struct event_header))
        return 0;

    if (message->header.length != count)
        return 0;

    return kernel_send(state->id, message->header.target, &self->states, message);

}

static unsigned int root_seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return 0;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_NORMAL, "event");

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

