struct node_operands
{

    unsigned int (*pick)(struct resource *resource, unsigned int source, struct message *message);
    unsigned int (*place)(struct resource *resource, unsigned int source, unsigned int event, unsigned int count, void *data);

};

struct node
{

    struct resource *resource;
    struct node_operands *operands;

};

void node_operands_init(struct node_operands *operands, unsigned int (*pick)(struct resource *resource, unsigned int source, struct message *message), unsigned int (*place)(struct resource *resource, unsigned int source, unsigned int event, unsigned int count, void *data));
void node_reset(struct node *node, struct resource *resource, struct node_operands *operands);
void node_init(struct node *node);
