#include <fudge.h>
#include <kernel.h>
#include <system/system.h>

static struct system_node root;
static struct system_node endpoint0;
static struct system_node endpoint1;

static unsigned int endpoint0_close(struct system_node *self)
{

    scheduler_detach(&endpoint0.mailboxes);
    scheduler_detachall(&endpoint1.mailboxes);

    return system_close(self);

}

static unsigned int endpoint0_read(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return scheduler_readpipe(&endpoint0.mailboxes, size * count, buffer);

}

static unsigned int endpoint0_write(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return scheduler_sendpipe(&endpoint1.mailboxes, size * count, buffer);

}

static unsigned int endpoint1_close(struct system_node *self)
{

    scheduler_detach(&endpoint1.mailboxes);
    scheduler_detachall(&endpoint0.mailboxes);

    return system_close(self);

}

static unsigned int endpoint1_read(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return scheduler_readpipe(&endpoint1.mailboxes, size * count, buffer);

}

static unsigned int endpoint1_write(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return scheduler_sendpipe(&endpoint0.mailboxes, size * count, buffer);

}

void module_init()
{

    system_initnode(&endpoint0, SYSTEM_NODETYPE_MAILBOX, "0");

    endpoint0.close = endpoint0_close;
    endpoint0.read = endpoint0_read;
    endpoint0.write = endpoint0_write;

    system_initnode(&endpoint1, SYSTEM_NODETYPE_MAILBOX, "1");

    endpoint1.close = endpoint1_close;
    endpoint1.read = endpoint1_read;
    endpoint1.write = endpoint1_write;

    system_initnode(&root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "pipe");
    system_addchild(&root, &endpoint0);
    system_addchild(&root, &endpoint1);

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

