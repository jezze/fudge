struct node_operands 
{

    unsigned int (*pick)(struct resource *resource, unsigned int source, struct message *message, unsigned int count, void *data);
    unsigned int (*place)(struct resource *resource, unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data);

};

struct node
{

    struct resource *resource;
    char *name;
    unsigned int namehash;
    struct node_operands *operands;
    struct list links;

};

void node_operands_init(struct node_operands *operands, unsigned int (*pick)(struct resource *resource, unsigned int source, struct message *message, unsigned int count, void *data), unsigned int (*place)(struct resource *resource, unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data));
void node_reset(struct node *node, char *name, struct resource *resource, struct node_operands *operands);
void node_init(struct node *node);
