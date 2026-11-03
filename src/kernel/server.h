struct server
{

    struct resource resource;
    struct resource_operands operands;

};

void server_init(struct server *server, void *data, unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data), unsigned long (*locate)(unsigned int id));
