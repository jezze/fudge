#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/event/event.h>

static struct system_node root;
static struct system_node event;
static struct system_node data;
static struct list eventlinks;
static char ringbuffer[FUDGE_BSIZE];
static struct ring ring;

static unsigned int event_open(struct system_node *self, struct service_state *state)
{

    list_add(&eventlinks, &state->link);

    return state->id;

}

static unsigned int event_close(struct system_node *self, struct service_state *state)
{

    list_remove(&eventlinks, &state->link);

    return state->id;

}

static unsigned int event_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return task_read(state->link.data, buffer, count);

}

static unsigned int event_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return event_send(&eventlinks, state, buffer, count);

}

static unsigned int data_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return ring_read(&ring, buffer, count);

}

static unsigned int data_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return ring_write(&ring, buffer, count);

}

void module_init(void)
{

    ring_init(&ring, FUDGE_BSIZE, ringbuffer);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "wm");
    system_initnode(&event, SYSTEM_NODETYPE_NORMAL, "event");
    system_initnode(&data, SYSTEM_NODETYPE_NORMAL, "data");

    event.open = event_open;
    event.close = event_close;
    event.read = event_read;
    event.write = event_write;
    data.read = data_read;
    data.write = data_write;

    system_addchild(&root, &event);
    system_addchild(&root, &data);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

