#define CONTAINER_SERVERS               32
#define CONTAINER_MOUNTS                32

struct container_node
{

    struct service_server *server;
    unsigned int id;

};

struct container_mount
{

    struct resource resource;
    struct container_node parent;
    struct container_node child;

};

struct container
{

    unsigned int id;
    struct resource resource;
    struct service_server servers[CONTAINER_SERVERS];
    unsigned int nservers;
    struct container_mount mounts[CONTAINER_MOUNTS];
    unsigned int nmounts;

};

struct service_server *container_getserver(struct container *container, unsigned int server);
struct container_mount *container_getmount(struct container *container, unsigned int mount);
void container_init(struct container *container, unsigned int id);
