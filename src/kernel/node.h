struct node
{

    struct resource *resource;
    char *name;
    unsigned int namehash;
    struct service *service;
    struct list links;

};

void node_reset(struct node *node, char *name, struct resource *resource, struct service *service);
void node_init(struct node *node);
