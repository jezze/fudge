#define RUNTIME_TASK_DESCRIPTOR_SLOTS   16
#define RUNTIME_CONTAINER_MOUNT_SLOTS   32
#define RUNTIME_TASKADDRESS_PHYSICAL    0x00400000
#define RUNTIME_TASKADDRESS_VIRTUAL     0x08048000
#define RUNTIME_TASKADDRESS_SIZE        0x00010000
#define RUNTIME_STACKADDRESS_PHYSICAL   0x00810000
#define RUNTIME_STACKADDRESS_VIRTUAL    0x80000000
#define RUNTIME_STACKADDRESS_SIZE       0x00010000

struct runtime_descriptor
{

    struct vfs_session session;
    unsigned int id;

};

enum runtime_task_state
{

    RUNTIME_TASK_STATE_USED             = 1,
    RUNTIME_TASK_STATE_IDLE             = 2

};

struct runtime_task
{

    unsigned int state;
    struct {unsigned int ip; unsigned int sp; unsigned int fp; unsigned int status;} registers;
    struct runtime_descriptor descriptors[RUNTIME_TASK_DESCRIPTOR_SLOTS];

};

struct runtime_mount
{

    struct runtime_descriptor parent;
    struct runtime_descriptor child;

};

struct runtime_container
{

    struct runtime_task *running;
    struct runtime_mount mounts[RUNTIME_CONTAINER_MOUNT_SLOTS];

};

struct runtime_descriptor *runtime_get_descriptor(struct runtime_task *task, unsigned int index);
struct runtime_mount *runtime_get_mount(struct runtime_container *container, unsigned int index);
struct runtime_descriptor *runtime_get_child(struct runtime_container *container, struct runtime_descriptor *descriptor);
struct runtime_descriptor *runtime_get_parent(struct runtime_container *container, struct runtime_descriptor *descriptor);
void runtime_init_task(struct runtime_task *task);
void runtime_init_container(struct runtime_container *container, struct runtime_task *task);
