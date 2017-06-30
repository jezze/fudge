#define CONTAINER_SERVERS               32
#define CONTAINER_MOUNTS                32

struct container_node
{

    struct service_server *server;
    unsigned int id;

};

struct container_mount
{

    struct list_item item;
    struct resource resource;
    struct container_node parent;
    struct container_node child;

};

struct container
{

    unsigned int id;
    struct resource resource;
    struct service_server servers[CONTAINER_SERVERS];
    struct list usedservers;
    struct list freeservers;
    struct container_mount mounts[CONTAINER_MOUNTS];
    struct list usedmounts;
    struct list freemounts;

};

struct service_server *container_getserver(struct container *container, unsigned int server);
struct service_server *container_getfreeserver(struct container *container);
void container_useserver(struct container *container, struct service_server *server);
void container_unuseserver(struct container *container, struct service_server *server);
struct container_mount *container_getmount(struct container *container, unsigned int mount);
struct container_mount *container_getfreemount(struct container *container);
void container_usemount(struct container *container, struct container_mount *mount);
void container_unusemount(struct container *container, struct container_mount *mount);
void container_init(struct container *container, unsigned int id);
