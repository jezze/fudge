#define CONTAINER_CHANNELS              16
#define CONTAINER_MOUNTS                32
#define CONTAINER_DESCRIPTORS           32 * 64

struct container_mount
{

    struct resource resource;
    struct {unsigned int channel; unsigned int id;} parent;
    struct {unsigned int channel; unsigned int id;} child;

};

struct container_descriptor
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
    struct container_descriptor descriptors[CONTAINER_DESCRIPTORS];

};

void container_init(struct container *container);
