#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "node.h"

void node_operands_init(struct node_operands *operands, unsigned int (*pick)(struct resource *resource, unsigned int source, struct message *message, unsigned int count, void *data), unsigned int (*place)(struct resource *resource, unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data))
{

    operands->pick = pick;
    operands->place = place;

}

void node_reset(struct node *node, char *name, struct resource *resource, struct node_operands *operands)
{

    node->resource = resource;
    node->operands = operands;
    node->name = name;
    node->namehash = (name) ? djb_hash(cstring_length(name), name) : 0;

}

void node_init(struct node *node)
{

    list_init(&node->links);
    node_reset(node, 0, 0, 0);

}

