struct node
{

    unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data);

};

void node_init(struct node *node, unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data));
