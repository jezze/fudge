#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node poll;
static struct system_node send;
static struct list polllinks;

static unsigned int poll_open(struct system_node *self, struct service_state *state)
{

    list_add(&polllinks, &state->link);

    return state->id;

}

static unsigned int poll_close(struct system_node *self, struct service_state *state)
{

    list_remove(&polllinks, &state->link);

    return state->id;

}

static unsigned int poll_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return task_read(state->link.data, buffer, count);

}

static unsigned int send_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    kernel_multicast(&polllinks, buffer, count);

    return count;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "wm");
    system_initnode(&poll, SYSTEM_NODETYPE_NORMAL, "poll");
    system_initnode(&send, SYSTEM_NODETYPE_NORMAL, "send");

    poll.open = poll_open;
    poll.close = poll_close;
    poll.read = poll_read;
    send.write = send_write;

    system_addchild(&root, &poll);
    system_addchild(&root, &send);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

