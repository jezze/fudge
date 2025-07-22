struct node
{

    struct list links;
    struct resource *resource;
    struct service *service;
    unsigned int namehash;

};

void node_reset(struct node *node, struct resource *resource, struct service *service);
void node_init(struct node *node);
