#define CONTAINER_CHANNELS              16
#define CONTAINER_MOUNTS                32

struct container
{

    struct resource resource;
    struct vfs_channel channels[CONTAINER_CHANNELS];
    struct vfs_mount mounts[CONTAINER_MOUNTS];

};

void container_init(struct container *container);
