#include <fudge.h>
#include "resource.h"
#include "node.h"

void node_reset(struct node *node, struct resource *resource, struct service *service)
{

    node->resource = resource;
    node->service = service;

}

void node_init(struct node *node)
{

    list_init(&node->links);

    node->resource = 0;
    node->service = 0;

}

