#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <event/event.h>
#include "console.h"

static struct system_node root;

void console_notify(struct console_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->mailboxes, count, buffer);
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

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    scheduler_mailboxes_addactive(&node->interface->mailboxes);

    return system_open(self);

}

static unsigned int interfacenode_inclose(struct system_node *self)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    scheduler_mailboxes_removeactive(&node->interface->mailboxes);

    return system_close(self);

}

static unsigned int interfacenode_inread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    return scheduler_mailboxes_readactive(&node->interface->mailboxes, count, buffer);

}

static unsigned int interfacenode_outwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    return node->interface->send(offset, count, buffer);

}

void console_registerinterface(struct console_interface *interface, unsigned int id)
{

    base_registerinterface(&interface->base, id);
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

void console_initinterface(struct console_interface *interface, struct base_driver *driver, unsigned int (*send)(unsigned int offset, unsigned int count, void *buffer))
{

    base_initinterface(&interface->base, driver);
    list_init(&interface->mailboxes);
    ctrl_init_consolesettings(&interface->settings);

    interface->send = send;

    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->node.in, SYSTEM_NODETYPE_NORMAL, "in");
    system_initnode(&interface->node.out, SYSTEM_NODETYPE_NORMAL, "out");

    interface->node.interface = interface;
    interface->node.ctrl.read = interfacenode_ctrlread;
    interface->node.ctrl.write = interfacenode_ctrlwrite;
    interface->node.in.open = interfacenode_inopen;
    interface->node.in.close = interfacenode_inclose;
    interface->node.in.read = interfacenode_inread;
    interface->node.out.write = interfacenode_outwrite;

}

void module_init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "console");

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

