#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "part.h"

static struct system_node root;
static struct system_node clone;

static unsigned int clone_child(struct system_node *self, char *path, unsigned int length)
{

    struct list_item *current;

    for (current = root.children.head; current; current = current->next)
    {

        struct system_node *node = current->data;
        struct part *part = current->data;

        if (node == self)
            continue;

        if (part->root.refcount)
            continue;

        return node->child(node, path, length);

    }

    return 0;

}

void part_init(struct part *part)
{

    system_initnode(&part->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "part");
    system_initnode(&part->data, SYSTEM_NODETYPE_NORMAL, "data");

}

void part_register(struct part *part)
{

    system_addchild(&part->root, &part->data);
    system_addchild(&root, &part->root);

}

void part_unregister(struct part *part)
{

    system_removechild(&part->root, &part->data);
    system_removechild(&root, &part->root);

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "part");
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

