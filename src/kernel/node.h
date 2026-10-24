struct node_operands
{

    unsigned int (*pick)(unsigned int source, struct message *messages, unsigned int count, unsigned int release, unsigned int flags);
    unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data);
    unsigned long (*locate)(unsigned int id);

};

void node_operands_init(struct node_operands *operands, unsigned int (*pick)(unsigned int source, struct message *messages, unsigned int count, unsigned int release, unsigned int flags), unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data), unsigned long (*locate)(unsigned int id));
