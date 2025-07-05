struct service
{

    struct resource resource;
    char *name;
    unsigned int hashname;
    struct resource *(*foreach)(struct resource *current);
    unsigned int (*getinode)(struct resource *current, unsigned int index);

};

void service_register(struct service *service);
void service_init(struct service *service, char *name, struct resource *(*foreach)(struct resource *current), unsigned int (*getinode)(struct resource *current, unsigned int index));
