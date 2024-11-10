struct service
{

    struct resource resource;
    char *name;
    unsigned int (*match)(unsigned int count, char *name);

};

void service_register(struct service *service);
void service_init(struct service *service, char *name, unsigned int (*match)(unsigned int count, char *name));
