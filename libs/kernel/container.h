#define CONTAINER_MOUNTS                32
#define CONTAINER_CALLS                 16

enum container_call
{

    CONTAINER_CALL_OPEN                 = 0x01,
    CONTAINER_CALL_CLOSE                = 0x02,
    CONTAINER_CALL_READ                 = 0x03,
    CONTAINER_CALL_WRITE                = 0x04,
    CONTAINER_CALL_MOUNT                = 0x05,
    CONTAINER_CALL_BIND                 = 0x06,
    CONTAINER_CALL_EXECUTE              = 0x07,
    CONTAINER_CALL_EXIT                 = 0x08,
    CONTAINER_CALL_LOAD                 = 0x09,
    CONTAINER_CALL_UNLOAD               = 0x0A,
    CONTAINER_CALL_SPAWN                = 0x0B

};

struct container_mount
{

    struct {struct vfs_session session; unsigned int id;} parent, child;

};

struct container
{

    struct container_mount mounts[CONTAINER_MOUNTS];
    unsigned int (*calls[CONTAINER_CALLS])(struct container *self, struct task *task, void *stack);

};

void container_init(struct container *container);
