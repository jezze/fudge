#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/ipv4/ipv4.h>
#include <modules/udp/udp.h>
#include "con.h"

static struct system_node root;
static struct system_node clone;

static struct ethernet_interface *findinterface(unsigned int index)
{

    struct resource *resource = 0;

    while ((resource = resource_findtype(resource, RESOURCE_ETHERNETINTERFACE)))
    {

        if (!index)
            return resource->data;

        index--;

    }

    return 0;

}

static struct ipv4_protocol *findprotocol(unsigned int id)
{

    struct resource *resource = 0;

    while ((resource = resource_findtype(resource, RESOURCE_IPV4PROTOCOL)))
    {

        struct ipv4_protocol *protocol = resource->data;

        if (protocol->id == id)
            return protocol;

    }

    return 0;

}

static unsigned int clone_child(struct system_node *self, char *path, unsigned int length)
{

    struct list_item *current;

    for (current = root.children.head; current; current = current->next)
    {

        struct system_node *node = current->data;

        if (node == self)
            continue;

        return node->child(node, path, length);

    }

    return 0;

}

static unsigned int ctrl_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct con *con = (struct con *)self->parent;

    return memory_read(buffer, count, &con->settings, sizeof (struct ctrl_consettings), state->offset);

}

static unsigned int ctrl_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct con *con = (struct con *)self->parent;

    count = memory_write(&con->settings, sizeof (struct ctrl_consettings), buffer, count, state->offset);
    con->interface = findinterface(con->settings.interface);
    con->protocol = findprotocol(con->settings.protocol);

    return count;

}

void con_init(struct con *con)
{

    ctrl_setconsettings(&con->settings, 0, 0, 0);
    system_initnode(&con->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "con");
    system_initnode(&con->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&con->data, SYSTEM_NODETYPE_MAILBOX, "data");

    con->ctrl.read = ctrl_read;
    con->ctrl.write = ctrl_write;

}

void con_register(struct con *con)
{

    system_addchild(&con->root, &con->ctrl);
    system_addchild(&con->root, &con->data);
    system_addchild(&root, &con->root);

}

void con_unregister(struct con *con)
{

    system_removechild(&con->root, &con->data);
    system_removechild(&root, &con->root);

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "con");
    system_initnode(&clone, SYSTEM_NODETYPE_GROUP, "clone");

    clone.child = clone_child;

    system_addchild(&root, &clone);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

