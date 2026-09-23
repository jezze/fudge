struct service
{

    struct resource resource;
    char name[32];
    unsigned int namehash;
    unsigned int inode;
    struct list links;

};

void service_setname(struct service *service, char *name);
void service_register(struct service *service, unsigned int inode);
void service_unregister(struct service *service);
void service_init(struct service *service, char *name);
