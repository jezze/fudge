#include <fudge.h>
#include "resource.h"
#include "node.h"

void node_init(struct node *node, struct resource *resource, unsigned int imailbox, unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data))
{

    list_init(&node->links);

    node->resource = resource;
    node->imailbox = imailbox;
    node->place = place;

}

