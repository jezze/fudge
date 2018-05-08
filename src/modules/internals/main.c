#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node nodes[512];

static void findall()
{

    struct resource *current = 0;
    unsigned int n = 0;

    while ((current = resource_foreachtype(current, RESOURCE_SERVICEBACKEND)))
    {

        struct service_backend *backend = current->data;

        system_initresourcenode(&nodes[n], SYSTEM_NODETYPE_NORMAL | SYSTEM_NODETYPE_MULTI, "backend", &backend->resource);
        system_addchild(&root, &nodes[n]);

        n++;

    }

    while ((current = resource_foreachtype(current, RESOURCE_SERVICEPROTOCOL)))
    {

        struct service_backend *backend = current->data;

        system_initresourcenode(&nodes[n], SYSTEM_NODETYPE_NORMAL | SYSTEM_NODETYPE_MULTI, "protocol", &backend->resource);
        system_addchild(&root, &nodes[n]);

        n++;

    }

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "internals");

    findall();

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

