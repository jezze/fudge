#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node data;
static char ringbuffer[FUDGE_BSIZE];
static struct ring ring;

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
    system_initnode(&data, SYSTEM_NODETYPE_NORMAL, "data");

    data.read = data_read;
    data.write = data_write;

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

