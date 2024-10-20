struct node
{

    struct list links;
    void *interface;
    unsigned int (*place)(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data);

};

void node_init(struct node *node, void *interface, unsigned int (*place)(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data));
