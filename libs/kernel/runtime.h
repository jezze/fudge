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

    struct vfs_interface *interface;
    unsigned int id;

};

struct runtime_task
{

    struct runtime_container *container;
    struct {unsigned int used; unsigned int idle;} status;
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
    void (*notify_interrupt)(struct runtime_container *self, unsigned int index);

};

struct runtime_descriptor *runtime_get_descriptor(struct runtime_task *task, unsigned int index);
struct runtime_mount *runtime_get_mount(struct runtime_task *task, unsigned int index);
unsigned int runtime_update_descriptor(struct runtime_task *task, struct runtime_descriptor *descriptor, struct vfs_interface *interface, unsigned int id, unsigned int count, const char *path);
void runtime_init_task(struct runtime_task *task, struct runtime_container *container);
void runtime_init_container(struct runtime_container *container);
