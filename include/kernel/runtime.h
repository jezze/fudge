#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

#define RUNTIME_TASK_SLOTS 8
#define RUNTIME_TASK_DESCRIPTOR_SLOTS 16
#define RUNTIME_TASK_ADDRESS_BASE 0x00300000
#define RUNTIME_TASK_ADDRESS_SIZE 0x00010000

struct runtime_registers
{

    unsigned int ip;
    unsigned int sp;
    unsigned int sb;

};

struct runtime_descriptor
{

    unsigned int id;
    struct vfs_node *node;
    unsigned int permissions;

};

struct runtime_task
{

    unsigned int id;
    unsigned int parentid;
    unsigned int used;
    struct runtime_registers registers;
    struct mmu_memory memory;
    unsigned int (*load)(struct runtime_task *self, void *entry, unsigned int argc, char **argv);
    void (*unload)(struct runtime_task *self);
    struct runtime_descriptor descriptors[RUNTIME_TASK_DESCRIPTOR_SLOTS];
    unsigned int (*get_descriptor_slot)(struct runtime_task *self);
    struct runtime_descriptor *(*get_descriptor)(struct runtime_task *self, unsigned int index);
    void (*set_descriptor)(struct runtime_task *self, unsigned int index, struct vfs_node *node);

};

extern unsigned int runtime_get_slot();
extern struct runtime_task *runtime_get_task(unsigned int index);
extern struct runtime_task *runtime_get_running_task();
extern void runtime_activate(struct runtime_task *task, struct runtime_task *ptask);
extern void runtime_get_state(unsigned int *ip, unsigned int *sp, unsigned int *sb);
extern void runtime_set_state(unsigned int ip, unsigned int sp, unsigned int sb);
extern void runtime_descriptor_init(struct runtime_descriptor *descriptor, unsigned int id, struct vfs_node *node, unsigned int permissions);
extern void runtime_task_init(struct runtime_task *task, unsigned int id);
extern void runtime_init();

#endif

