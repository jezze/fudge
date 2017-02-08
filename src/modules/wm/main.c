#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node data;
static struct system_node send;
static char ringbuffer[FUDGE_BSIZE];
static struct ring ring;

static unsigned int data_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return ring_read(&ring, buffer, count);

}

static unsigned int send_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return ring_write(&ring, buffer, count);

}

void module_init(void)
{

    ring_init(&ring, FUDGE_BSIZE, ringbuffer);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "wm");
    system_initnode(&data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&send, SYSTEM_NODETYPE_NORMAL, "send");

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

