#define CONTAINER_CHANNELS              16
#define CONTAINER_MOUNTS                32
#define CONTAINER_SESSIONS              32 * 64

struct container_mount
{

    struct resource resource;
    struct {unsigned int channel; unsigned int id;} parent;
    struct {unsigned int channel; unsigned int id;} child;

};

struct container_session
{

    struct resource resource;
    unsigned int channel;
    unsigned int id;
    unsigned int offset;

};

struct container
{

    struct resource resource;
    struct service_channel channels[CONTAINER_CHANNELS];
    struct container_mount mounts[CONTAINER_MOUNTS];
    struct container_session sessions[CONTAINER_SESSIONS];

};

void container_init(struct container *container);
