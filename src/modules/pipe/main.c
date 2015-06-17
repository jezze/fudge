#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node endpoint0;
static struct system_node endpoint1;

static unsigned int readpipe(struct list *mailboxes, unsigned int size, unsigned int count, void *buffer)
{

    if (mailboxes->head != 0)
        return scheduler_readactive(size, count, buffer);
    else
        return 0;

}

static unsigned int sendpipe(struct list *mailboxes, unsigned int size, unsigned int count, void *buffer)
{

    scheduler_sendlist(mailboxes, size, count, buffer);

    return count;

}

static unsigned int endpoint0_close(struct system_node *self)
{

    scheduler_detachactive(&endpoint0.mailboxes);
    scheduler_detachlist(&endpoint1.mailboxes);

    return system_close(self);

}

static unsigned int endpoint0_read(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return readpipe(&endpoint0.mailboxes, size, count, buffer);

}

static unsigned int endpoint0_write(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return sendpipe(&endpoint1.mailboxes, size, count, buffer);

}

static unsigned int endpoint1_close(struct system_node *self)
{

    scheduler_detachactive(&endpoint1.mailboxes);
    scheduler_detachlist(&endpoint0.mailboxes);

    return system_close(self);

}

static unsigned int endpoint1_read(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return readpipe(&endpoint1.mailboxes, size, count, buffer);

}

static unsigned int endpoint1_write(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return sendpipe(&endpoint0.mailboxes, size, count, buffer);

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

