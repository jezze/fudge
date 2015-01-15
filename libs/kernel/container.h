#define CONTAINER_CHANNELS              16
#define CONTAINER_MOUNTS                32
#define CONTAINER_CALLS                 16

enum container_call
{

    CONTAINER_CALL_WALK                 = 0x01,
    CONTAINER_CALL_CREATE               = 0x02,
    CONTAINER_CALL_DESTROY              = 0x03,
    CONTAINER_CALL_OPEN                 = 0x04,
    CONTAINER_CALL_CLOSE                = 0x05,
    CONTAINER_CALL_READ                 = 0x06,
    CONTAINER_CALL_WRITE                = 0x07,
    CONTAINER_CALL_AUTH                 = 0x08,
    CONTAINER_CALL_MOUNT                = 0x09,
    CONTAINER_CALL_BIND                 = 0x0A,
    CONTAINER_CALL_EXECUTE              = 0x0B,
    CONTAINER_CALL_LOAD                 = 0x0C,
    CONTAINER_CALL_UNLOAD               = 0x0D,
    CONTAINER_CALL_SPAWN                = 0x0E,
    CONTAINER_CALL_DESPAWN              = 0x0F

};

struct container
{

    struct resource resource;
    struct vfs_channel channels[CONTAINER_CHANNELS];
    struct vfs_mount mounts[CONTAINER_MOUNTS];
    unsigned int (*calls[CONTAINER_CALLS])(struct container *self, struct task *task, void *stack);

};

void container_init(struct container *container, unsigned int (*spawn)(struct container *self, struct task *task, void *stack), unsigned int (*despawn)(struct container *self, struct task *task, void *stack));
