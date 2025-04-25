struct node
{

    struct list links;
    struct resource *resource;
    unsigned int imailbox;
    unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data);

};

void node_init(struct node *node, struct resource *resource, unsigned int imailbox, unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data));
