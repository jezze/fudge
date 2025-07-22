struct service
{

    struct resource resource;
    char *name;
    unsigned int namehash;
    struct resource *(*foreach)(struct resource *current);
    unsigned int (*getinode)(struct resource *current, unsigned int index);
    unsigned int (*getinodename)(unsigned int namehash);
    unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data);

};

void service_register(struct service *service);
void service_init(struct service *service, char *name, struct resource *(*foreach)(struct resource *current), unsigned int (*getinode)(struct resource *current, unsigned int index), unsigned int (*getinodename)(unsigned int namehash), unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data));
