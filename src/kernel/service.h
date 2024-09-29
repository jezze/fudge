struct service
{

    struct resource resource;
    char *name;
    struct node *(*match)(unsigned int count, char *name);

};

void service_register(struct service *service);
void service_init(struct service *service, char *name, struct node *(*match)(unsigned int count, char *name));
