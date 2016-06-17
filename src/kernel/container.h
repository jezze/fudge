#define CONTAINER_MOUNTS                32

struct container_mount
{

    struct resource resource;
    struct {struct service_backend *backend; struct service_protocol *protocol; unsigned int id;} parent;
    struct {struct service_backend *backend; struct service_protocol *protocol; unsigned int id;} child;

};

struct container
{

    struct resource resource;
    struct container_mount mounts[CONTAINER_MOUNTS];

};

void container_init(struct container *container);
