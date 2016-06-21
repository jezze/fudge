#define CONTAINER_SERVERS               32
#define CONTAINER_MOUNTS                32

struct container_server
{

    struct service_backend *backend;
    struct service_protocol *protocol;

};

struct container_mount
{

    struct resource resource;
    struct {struct container_server *server; unsigned int id;} parent;
    struct {struct container_server *server; unsigned int id;} child;

};

struct container
{

    struct resource resource;
    struct container_server servers[CONTAINER_SERVERS];
    unsigned int nservers;
    struct container_mount mounts[CONTAINER_MOUNTS];
    unsigned int nmounts;

};

void container_init(struct container *container);
