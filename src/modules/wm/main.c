#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node data;
static struct system_node send;
static struct list datalinks;

static unsigned int data_open(struct system_node *self, struct service_state *state)
{

    list_add(&datalinks, &state->link);

    return state->id;

}

static unsigned int data_close(struct system_node *self, struct service_state *state)
{

    list_remove(&datalinks, &state->link);

    return state->id;

}

static unsigned int data_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return task_read(state->link.data, buffer, count);

}

static unsigned int send_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    kernel_multicast(&datalinks, buffer, count);

    return count;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "wm");
    system_initnode(&data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&send, SYSTEM_NODETYPE_NORMAL, "send");

    data.open = data_open;
    data.close = data_close;
    data.read = data_read;
    send.write = send_write;

    system_addchild(&root, &data);
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

