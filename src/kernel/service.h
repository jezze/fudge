#define SERVICE_NAMESIZE                32
#define SERVICE_CHILDREN                32

struct service
{

    struct resource resource;
    char name[SERVICE_NAMESIZE];
    unsigned int namehash;
    unsigned int inode;
    struct service *parent;
    unsigned int index;
    struct list links;

};

void service_register(struct service *service, unsigned int inode, char *name);
void service_registerdirectory(struct service *service, char *name);
void service_registerchild(struct service *service, unsigned int inode, struct service *parent);
void service_unregister(struct service *service);
void service_init(struct service *service);
