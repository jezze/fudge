#include <fudge.h>
#include "resource.h"
#include "node.h"

void node_init(struct node *node, struct mailbox *mailbox, void *interface, unsigned int (*place)(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data))
{

    list_init(&node->children);

    node->mailbox = mailbox;
    node->interface = interface;
    node->place = place;

}

