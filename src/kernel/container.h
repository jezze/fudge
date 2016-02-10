#define CONTAINER_MOUNTS                32
#define CONTAINER_SESSIONS              32 * 64

struct container_mount
{

    struct resource resource;
    struct {struct service_backend *backend; struct service_protocol *protocol; unsigned int id;} parent;
    struct {struct service_backend *backend; struct service_protocol *protocol; unsigned int id;} child;

};

struct container_session
{

    struct resource resource;
    struct service_backend *backend;
    struct service_protocol *protocol;
    struct service_state state;

};

struct container
{

    struct resource resource;
    struct container_mount mounts[CONTAINER_MOUNTS];
    struct container_session sessions[CONTAINER_SESSIONS];

};

void container_init(struct container *container);
