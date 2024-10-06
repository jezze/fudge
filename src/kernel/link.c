#include <fudge.h>
#include "link.h"

void link_init(struct link *link, struct node *target, struct mailbox *mailbox)
{

    link->target = target;
    link->mailbox = mailbox;

}

