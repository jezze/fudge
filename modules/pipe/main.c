#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>

struct pipe_endpoint
{

    struct system_node node;

};

static struct list mailboxes;
static struct pipe_endpoint endpoint0;
static struct pipe_endpoint endpoint1;
static struct system_node root;

static unsigned int endpoint0_close(struct system_node *self)
{

    scheduler_mailboxes_removeall(&mailboxes);

    return system_close(self);

}

static unsigned int endpoint0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&mailboxes, count, buffer);

    return count;

}

static unsigned int endpoint1_open(struct system_node *self)
{

    scheduler_mailboxes_addactive(&mailboxes);

    return system_open(self);

}

static unsigned int endpoint1_close(struct system_node *self)
{

    scheduler_mailboxes_removeactive(&mailboxes);

    return system_close(self);

}

static unsigned int endpoint1_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return scheduler_mailboxes_readactive(&mailboxes, count, buffer);

}

void init()
{

    list_init(&mailboxes);
    system_initnode(&endpoint0.node, SYSTEM_NODETYPE_NORMAL, "0");

    endpoint0.node.close = endpoint0_close;
    endpoint0.node.write = endpoint0_write;

    system_initnode(&endpoint1.node, SYSTEM_NODETYPE_NORMAL, "1");

    endpoint1.node.open = endpoint1_open;
    endpoint1.node.close = endpoint1_close;
    endpoint1.node.read = endpoint1_read;

    system_initnode(&root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "pipe");
    system_addchild(&root, &endpoint0.node);
    system_addchild(&root, &endpoint1.node);
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

