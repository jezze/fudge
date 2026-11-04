#define SERVICE_NAMESIZE                32

struct service
{

    struct resource resource;
    char name[SERVICE_NAMESIZE];
    unsigned int inode;

};

void service_register(struct service *service, unsigned int inode);
void service_unregister(struct service *service);
void service_init(struct service *service, char *name);
