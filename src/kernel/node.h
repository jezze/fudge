struct node
{

    struct mailbox *mailbox;
    struct resource *resource;
    unsigned int (*place)(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data);

};

void node_init(struct node *node, struct mailbox *mailbox, struct resource *resource, unsigned int (*place)(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data));
