struct link
{

    struct node *target;
    struct mailbox *mailbox;

};

void link_init(struct link *link, struct node *target, struct mailbox *mailbox);
