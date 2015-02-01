#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <event/event.h>
#include "console.h"

static struct list mailboxes;
static struct system_node root;

void console_notify(unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&mailboxes, count, buffer);
    event_notify(EVENT_TYPE_CONSOLE, count, buffer);

}

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    return memory_read(buffer, count, &node->interface->settings, sizeof (struct ctrl_consolesettings), offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    return memory_write(&node->interface->settings, sizeof (struct ctrl_consolesettings), buffer, count, offset);

}

static unsigned int interfacenode_inopen(struct system_node *self)
{

    scheduler_mailboxes_addactive(&mailboxes);

    return system_open(self);

}

static unsigned int interfacenode_inclose(struct system_node *self)
{

    scheduler_mailboxes_removeactive(&mailboxes);

    return system_close(self);

}

static unsigned int interfacenode_inread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return scheduler_mailboxes_readactive(&mailboxes, count, buffer);

}

static unsigned int interfacenode_outwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    return node->interface->wout(offset, count, buffer);

}

void console_registerinterface(struct console_interface *interface, struct base_bus *bus, unsigned int id)
{

    base_registerinterface(&interface->base, bus, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.ctrl);
    system_addchild(&interface->node.base, &interface->node.in);
    system_addchild(&interface->node.base, &interface->node.out);

}

void console_unregisterinterface(struct console_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.ctrl);
    system_removechild(&interface->node.base, &interface->node.in);
    system_removechild(&interface->node.base, &interface->node.out);
    system_removechild(&root, &interface->node.base);

}

void console_initinterface(struct console_interface *interface, struct base_driver *driver, unsigned int (*wout)(unsigned int offset, unsigned int count, void *buffer))
{

    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->node.in, SYSTEM_NODETYPE_NORMAL, "in");
    system_initnode(&interface->node.out, SYSTEM_NODETYPE_NORMAL, "out");
    ctrl_init_consolesettings(&interface->settings);

    interface->wout = wout;
    interface->node.interface = interface;
    interface->node.ctrl.read = interfacenode_ctrlread;
    interface->node.ctrl.write = interfacenode_ctrlwrite;
    interface->node.in.open = interfacenode_inopen;
    interface->node.in.close = interfacenode_inclose;
    interface->node.in.read = interfacenode_inread;
    interface->node.out.write = interfacenode_outwrite;

}

void init()
{

    list_init(&mailboxes);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "console");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

