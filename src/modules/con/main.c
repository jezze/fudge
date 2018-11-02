#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "con.h"

static struct system_node root;

void con_init(struct con *con)
{

    resource_init(&con->resource, RESOURCE_CON, con);
    system_initnode(&con->root, SYSTEM_NODETYPE_MULTIGROUP, "con");
    system_initnode(&con->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&con->data, SYSTEM_NODETYPE_NORMAL, "data");

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

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

