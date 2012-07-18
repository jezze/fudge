#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

#define RUNTIME_TASK_SLOTS 32
#define RUNTIME_TASK_DESCRIPTOR_SLOTS 16
#define RUNTIME_TASK_MOUNT_SLOTS 8
#define RUNTIME_TASK_ADDRESS_BASE 0x00400000
#define RUNTIME_TASK_ADDRESS_SIZE 0x00010000

struct runtime_descriptor
{

    unsigned int id;
    struct modules_filesystem *filesystem;
    unsigned int permissions;

};

struct runtime_memory
{

    unsigned int paddress;
    unsigned int vaddress;
    unsigned int size;

};

struct runtime_mount
{

    unsigned int id;
    struct modules_filesystem *filesystem;
    unsigned int count;
    char path[64];

};

struct runtime_registers
{

    unsigned int ip;
    unsigned int sp;
    unsigned int sb;

};

struct runtime_task
{

    struct runtime_task *parent;
    unsigned int id;
    unsigned int used;
    unsigned int event;
    unsigned int wait;
    struct runtime_registers registers;
    struct runtime_memory memory;
    struct runtime_descriptor descriptors[RUNTIME_TASK_DESCRIPTOR_SLOTS];
    struct runtime_descriptor *(*get_descriptor)(struct runtime_task *self, unsigned int index);
    unsigned int (*get_descriptor_slot)(struct runtime_task *self);
    struct runtime_mount mounts[RUNTIME_TASK_MOUNT_SLOTS];
    struct runtime_mount *(*get_mount)(struct runtime_task *self, unsigned int index);
    struct runtime_mount *(*find_mount)(struct runtime_task *self, void *path);
    unsigned int (*get_mount_slot)(struct runtime_task *self);

};

struct runtime_task *runtime_get_task(unsigned int index);
unsigned int runtime_get_task_slot();
void runtime_descriptor_init(struct runtime_descriptor *descriptor, unsigned int id, struct modules_filesystem *filesystem, unsigned int permissions);
void runtime_memory_init(struct runtime_memory *memory, unsigned int paddress, unsigned int vaddress, unsigned int size);
void runtime_mount_init(struct runtime_mount *mount, unsigned int id, struct modules_filesystem *filesystem, unsigned int count, char *path);
void runtime_registers_init(struct runtime_registers *registers, unsigned int ip, unsigned int sp, unsigned int sb);
void runtime_task_init(struct runtime_task *task, unsigned int id);
void runtime_task_clone(struct runtime_task *task, struct runtime_task *original, unsigned int id);
struct runtime_task *runtime_schedule();
struct runtime_task *runtime_setup();

#endif

