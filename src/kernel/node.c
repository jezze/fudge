#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "node.h"

void node_operands_init(struct node_operands *operands, unsigned int (*pick)(unsigned int source, struct message *message), unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data))
{

    operands->pick = pick;
    operands->place = place;

}

