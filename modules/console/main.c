#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include <event/event.h>
#include "console.h"

static struct system_node root;

void console_notify(struct console_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->in.mailboxes, count, buffer);
    event_notify(EVENT_TYPE_CONSOLE, count, buffer);

}

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interface *interface = (struct console_interface *)self->parent;

    return memory_read(buffer, count, &interface->settings, sizeof (struct ctrl_consolesettings), offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interface *interface = (struct console_interface *)self->parent;

    return memory_write(&interface->settings, sizeof (struct ctrl_consolesettings), buffer, count, offset);

}

static unsigned int interfacenode_outwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interface *interface = (struct console_interface *)self->parent;

    return interface->send(offset, count, buffer);

}

void console_registerinterface(struct console_interface *interface, unsigned int id)
{

    system_registerinterface(&interface->base, id);
    system_addchild(&interface->base.root, &interface->ctrl);
    system_addchild(&interface->base.root, &interface->in);
    system_addchild(&interface->base.root, &interface->out);
    system_addchild(&root, &interface->base.root);

}

void console_unregisterinterface(struct console_interface *interface)
{

    system_unregisterinterface(&interface->base);
    system_removechild(&interface->base.root, &interface->ctrl);
    system_removechild(&interface->base.root, &interface->in);
    system_removechild(&interface->base.root, &interface->out);
    system_removechild(&root, &interface->base.root);

}

void console_initinterface(struct console_interface *interface, struct base_driver *driver, unsigned int (*send)(unsigned int offset, unsigned int count, void *buffer))
{

    system_initinterface(&interface->base, driver->name);
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->in, SYSTEM_NODETYPE_MAILBOX, "in");
    system_initnode(&interface->out, SYSTEM_NODETYPE_NORMAL, "out");
    ctrl_init_consolesettings(&interface->settings);

    interface->send = send;
    interface->ctrl.read = interfacenode_ctrlread;
    interface->ctrl.write = interfacenode_ctrlwrite;
    interface->out.write = interfacenode_outwrite;

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

