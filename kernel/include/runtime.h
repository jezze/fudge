#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

#define RUNTIME_TASK_DESCRIPTOR_SLOTS 16
#define RUNTIME_TASK_MOUNT_SLOTS 32
#define RUNTIME_TASK_ADDRESS_SIZE  0x00010000
#define RUNTIME_TASK_PADDRESS_BASE 0x00400000
#define RUNTIME_TASK_VADDRESS_BASE 0x08048000

struct runtime_mount
{

    struct vfs_interface *interface;
    unsigned int count;
    char path[64];

};

struct runtime_descriptor
{

    unsigned int id;
    struct vfs_interface *interface;

};

struct runtime_registers
{

    unsigned int ip;
    unsigned int sp;
    unsigned int sb;

};

struct runtime_status
{

    unsigned int used;
    unsigned int event;
    unsigned int idle;

};

struct runtime_task
{

    unsigned int id;
    struct runtime_status status;
    struct runtime_registers registers;
    struct runtime_descriptor descriptors[RUNTIME_TASK_DESCRIPTOR_SLOTS];
    struct runtime_mount mounts[RUNTIME_TASK_MOUNT_SLOTS];

};

struct runtime_descriptor *runtime_get_task_descriptor(struct runtime_task *task, unsigned int index);
struct runtime_mount *runtime_get_task_mount(struct runtime_task *task, unsigned int index);
struct runtime_mount *runtime_find_task_mount(struct runtime_task *self, char *path);
void runtime_init_descriptor(struct runtime_descriptor *descriptor, unsigned int id, struct vfs_interface *interface);
void runtime_init_mount(struct runtime_mount *mount, struct vfs_interface *interface, unsigned int count, char *path);
void runtime_init_task(struct runtime_task *task, unsigned int id, unsigned int ip);
void runtime_reset_task(struct runtime_task *task, unsigned int ip);

#endif

