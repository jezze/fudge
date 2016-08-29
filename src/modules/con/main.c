#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "con.h"

static struct system_node root;
static struct system_node clone;

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

void con_init(struct con *con)
{

    system_initnode(&con->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "con");
    system_initnode(&con->data, SYSTEM_NODETYPE_NORMAL, "data");

}

void con_register(struct con *con)
{

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

