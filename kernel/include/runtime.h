#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

#define RUNTIME_TASK_SLOTS 32
#define RUNTIME_TASK_DESCRIPTOR_SLOTS 16
#define RUNTIME_TASK_MOUNT_SLOTS 8
#define RUNTIME_TASK_ADDRESS_SIZE  0x00010000
#define RUNTIME_TASK_PADDRESS_BASE 0x00400000
#define RUNTIME_TASK_VADDRESS_BASE 0x08048000

struct runtime_mount
{

    unsigned int id;
    struct modules_filesystem *filesystem;
    unsigned int count;
    char path[64];

};

struct runtime_descriptor
{

    unsigned int id;
    struct runtime_mount *mount;

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
    void (*clone)(struct runtime_task *self, struct runtime_task *task, unsigned int id);
    struct runtime_descriptor descriptors[RUNTIME_TASK_DESCRIPTOR_SLOTS];
    struct runtime_descriptor *(*get_descriptor)(struct runtime_task *self, unsigned int index);
    struct runtime_mount mounts[RUNTIME_TASK_MOUNT_SLOTS];
    struct runtime_mount *(*get_mount)(struct runtime_task *self, unsigned int index);
    struct runtime_mount *(*find_mount)(struct runtime_task *self, char *path);

};

struct runtime_task *runtime_get_task(unsigned int index);
unsigned int runtime_get_task_slot();
void runtime_descriptor_init(struct runtime_descriptor *descriptor, unsigned int id, struct runtime_mount *mount);
void runtime_mount_init(struct runtime_mount *mount, unsigned int id, struct modules_filesystem *filesystem, unsigned int count, char *path);
void runtime_registers_init(struct runtime_registers *registers, unsigned int ip, unsigned int sp, unsigned int sb);
void runtime_task_init(struct runtime_task *task, unsigned int id);
struct runtime_task *runtime_schedule();
struct runtime_task *runtime_setup();

#endif

