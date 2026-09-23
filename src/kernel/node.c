#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "node.h"

void node_operands_init(struct node_operands *operands, unsigned int (*pick)(struct resource *resource, unsigned int source, struct message *message), unsigned int (*place)(struct resource *resource, unsigned int source, unsigned int event, unsigned int count, void *data))
{

    operands->pick = pick;
    operands->place = place;

}

void node_reset(struct node *node, struct resource *resource, struct node_operands *operands)
{

    node->resource = resource;
    node->operands = operands;

}

void node_init(struct node *node)
{

    node_reset(node, 0, 0);

}

