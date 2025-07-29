struct service
{

    unsigned int (*pick)(unsigned int source, struct message *message, unsigned int count, void *data);
    unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data);

};

void service_init(struct service *service, unsigned int (*pick)(unsigned int source, struct message *message, unsigned int count, void *data), unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data));
