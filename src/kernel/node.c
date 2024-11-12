#include <fudge.h>
#include "resource.h"
#include "node.h"

void node_init(struct node *node, unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data))
{

    node->place = place;

}

