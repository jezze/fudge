#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>

static struct system_node root;
static struct system_node endpoint0;
static struct system_node endpoint1;

static unsigned int endpoint0_close(struct system_node *self)
{

    scheduler_mailboxes_removeactive(&endpoint0.mailboxes);
    scheduler_mailboxes_removeall(&endpoint1.mailboxes);

    return system_close(self);

}

static unsigned int endpoint0_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return scheduler_mailboxes_send(&endpoint1.mailboxes, count, buffer);

}

static unsigned int endpoint1_close(struct system_node *self)
{

    scheduler_mailboxes_removeactive(&endpoint1.mailboxes);
    scheduler_mailboxes_removeall(&endpoint0.mailboxes);

    return system_close(self);

}

static unsigned int endpoint1_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return scheduler_mailboxes_send(&endpoint0.mailboxes, count, buffer);

}

void module_init()
{

    system_initnode(&endpoint0, SYSTEM_NODETYPE_MAILBOX, "0");

    endpoint0.close = endpoint0_close;
    endpoint0.write = endpoint0_write;

    system_initnode(&endpoint1, SYSTEM_NODETYPE_MAILBOX, "1");

    endpoint1.close = endpoint1_close;
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

