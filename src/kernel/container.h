#define CONTAINER_SERVERS               32
#define CONTAINER_MOUNTS                32

struct container_server
{

    struct service_backend *backend;
    struct service_protocol *protocol;

};

struct container_node
{

    struct container_server *server;
    unsigned int id;

};

struct container_mount
{

    struct resource resource;
    struct container_node parent;
    struct container_node child;

};

struct task_descriptor
{

    struct list_item link;
    struct task *task;
    struct container_server *server;
    unsigned int id;
    unsigned int offset;
    unsigned int current;

};

struct container
{

    struct resource resource;
    struct container_server servers[CONTAINER_SERVERS];
    unsigned int nservers;
    struct container_mount mounts[CONTAINER_MOUNTS];
    unsigned int nmounts;
    struct task_descriptor descriptors[4096];

};

void container_initdescriptor(struct task_descriptor *descriptor, struct task *task);
void container_init(struct container *container);
