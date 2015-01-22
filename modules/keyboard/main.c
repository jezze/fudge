#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>
#include <event/event.h>
#include "keyboard.h"

static struct list mailboxes;
static struct system_node root;

void keyboard_notify(unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = mailboxes.head; current; current = current->next)
    {

        struct task_mailbox *mailbox = current->data;

        scheduler_mailbox_write(mailbox, count, buffer);

    }

    event_notify(EVENT_TYPE_KEYBOARD, count, buffer);

}

static unsigned int interfacenode_dataopen(struct system_node *self)
{

    scheduler_activetask_addmailbox(&mailboxes);

    return system_open(self);

}

static unsigned int interfacenode_dataclose(struct system_node *self)
{

    scheduler_activetask_removemailbox(&mailboxes);

    return system_close(self);

}

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return scheduler_activetask_readmailbox(count, buffer);

}

void keyboard_registerinterface(struct keyboard_interface *interface, struct base_bus *bus, unsigned int id)
{

    base_registerinterface(&interface->base, bus, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.data);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.data);
    system_removechild(&root, &interface->node.base);

}

void keyboard_initinterface(struct keyboard_interface *interface, struct base_driver *driver)
{

    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_NORMAL, "data");

    interface->node.interface = interface;
    interface->node.data.open = interfacenode_dataopen;
    interface->node.data.close = interfacenode_dataclose;
    interface->node.data.read = interfacenode_dataread;

}

void init()
{

    list_init(&mailboxes);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "keyboard");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

