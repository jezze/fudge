struct service
{

    struct resource resource;
    unsigned int (*match)(unsigned int count, char *name);

};

void service_init(struct service *service, unsigned int (*match)(unsigned int count, char *name));
