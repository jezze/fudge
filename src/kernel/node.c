#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "node.h"

void node_reset(struct node *node, char *name, struct resource *resource, struct service *service)
{

    node->resource = resource;
    node->service = service;
    node->name = name;
    node->namehash = (name) ? djb_hash(cstring_length(name), name) : 0;

}

void node_init(struct node *node)
{

    list_init(&node->links);
    node_reset(node, 0, 0, 0);

}

