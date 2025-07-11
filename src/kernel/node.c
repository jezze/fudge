#include <fudge.h>
#include "resource.h"
#include "node.h"

void node_reset(struct node *node, struct resource *resource, unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data))
{

    node->resource = resource;
    node->place = place;

}

void node_init(struct node *node)
{

    list_init(&node->links);

    node->resource = 0;
    node->place = 0;

}

