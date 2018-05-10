#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "con.h"

static struct system_node root;
static struct system_node clone;

static struct system_node *clone_child(struct system_node *self, struct service_state *state, char *path, unsigned int length)
{

    struct list_item *current;

    spinlock_acquire(&root.children.spinlock);

    for (current = root.children.head; current; current = current->next)
    {

        struct system_node *node = current->data;

        if (node == self)
            continue;

        return node->operations.child(node, state, path, length);

    }

    spinlock_release(&root.children.spinlock);

    return self;

}

void con_init(struct con *con)
{

    ctrl_setconsettings(&con->settings, 0, 0, 0);
    resource_init(&con->resource, RESOURCE_CON, con);
    system_initnode(&con->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "con");
    system_initnode(&con->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&con->data, SYSTEM_NODETYPE_MAILBOX, "data");

}

void con_register(struct con *con)
{

    system_addchild(&con->root, &con->ctrl);
    system_addchild(&con->root, &con->data);
    system_addchild(&root, &con->root);

}

void con_unregister(struct con *con)
{

    system_removechild(&con->root, &con->ctrl);
    system_removechild(&con->root, &con->data);
    system_removechild(&root, &con->root);

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "con");
    system_initnode(&clone, SYSTEM_NODETYPE_GROUP, "clone");

    clone.operations.child = clone_child;

}

void module_register(void)
{

    system_registernode(&root);
    system_addchild(&root, &clone);

}

void module_unregister(void)
{

    system_unregisternode(&root);
    system_removechild(&root, &clone);

}

