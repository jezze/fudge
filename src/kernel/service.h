#define SERVICE_NAMESIZE                32
#define SERVICE_CHILDREN                32

struct service
{

    struct resource resource;
    char name[SERVICE_NAMESIZE];
    unsigned int namehash;
    unsigned int inode;
    unsigned int children[SERVICE_CHILDREN];

};

void service_register(struct service *service, unsigned int inode, char *name);
void service_registerdirectory(struct service *service, char *name);
void service_addchild(struct service *service, unsigned int inode);
void service_removechild(struct service *service, unsigned int inode);
void service_unregister(struct service *service);
void service_init(struct service *service);
