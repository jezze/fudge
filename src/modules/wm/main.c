#include <fudge.h>
#include <kernel.h>
#include <event/event.h>
#include <modules/system/system.h>
#include <modules/event/event.h>

static struct system_node root;
static struct system_node data;
static struct system_node event;
static struct list eventstates;
static char databuffer[FUDGE_BSIZE];
static struct ring dataring;

static unsigned int data_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return ring_read(&dataring, buffer, count);

}

static unsigned int data_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return ring_write(&dataring, buffer, count);

}

static unsigned int event_open(struct system_node *self, struct service_state *state)
{

    list_add(&eventstates, &state->item);

    return (unsigned int)self;

}

static unsigned int event_close(struct system_node *self, struct service_state *state)
{

    list_remove(&eventstates, &state->item);

    return (unsigned int)self;

}

static unsigned int event_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return event_send(&eventstates, state, buffer, count);

}

void module_init(void)
{

    ring_init(&dataring, FUDGE_BSIZE, databuffer);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "wm");
    system_initnode(&data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&event, SYSTEM_NODETYPE_NORMAL, "event");

    data.read = data_read;
    data.write = data_write;
    event.open = event_open;
    event.close = event_close;
    event.write = event_write;

    system_addchild(&root, &data);
    system_addchild(&root, &event);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

