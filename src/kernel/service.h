#define SERVICE_NAMESIZE                32

struct service
{

    struct resource resource;
    char name[SERVICE_NAMESIZE];
    unsigned int namehash;
    unsigned int inode;
    unsigned int type;
    struct node_operands *operands;

};

void service_register(struct service *service, unsigned int inode, char *name);
void service_registernode(struct service *service, char *name, struct node_operands *operands);
void service_registerdirectory(struct service *service, char *name, unsigned int type, struct node_operands *operands);
void service_unregister(struct service *service);
void service_init(struct service *service);
void service_setup(void);
