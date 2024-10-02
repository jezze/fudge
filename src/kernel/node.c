#include <fudge.h>
#include "resource.h"
#include "node.h"

void node_init(struct node *node, void *interface, unsigned int (*place)(struct node *source, struct node *target, unsigned int ichannel, unsigned int event, unsigned int count, void *data))
{

    list_init(&node->links);

    node->interface = interface;
    node->ichannel = 0;
    node->place = place;

}

