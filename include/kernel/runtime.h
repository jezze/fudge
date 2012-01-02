#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

#define RUNTIME_TASK_SLOTS 8
#define RUNTIME_TASK_DESCRIPTOR_SLOTS 16

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
    struct runtime_descriptor *(*add_descriptor)(struct runtime_task *self, struct vfs_node *node);
    struct runtime_descriptor *(*get_descriptor)(struct runtime_task *self, unsigned int index);
    void (*remove_descriptor)(struct runtime_task *self, unsigned int index);

};

struct runtime_control
{

    struct runtime_task tasks[RUNTIME_TASK_SLOTS];
    struct runtime_task *running;

};

extern unsigned int runtime_get_free_slot();
extern struct runtime_task *runtime_get_task(unsigned int id);
extern struct runtime_task *runtime_get_running_task();
extern void runtime_set_state(unsigned int ip, unsigned int sp, unsigned int sb);
extern void runtime_get_state(unsigned int *ip, unsigned int *sp, unsigned int *sb);
extern void runtime_activate(struct runtime_task *task);
extern void runtime_descriptor_init(struct runtime_descriptor *descriptor, unsigned int id, struct vfs_node *node, unsigned int permissions);
extern void runtime_task_init(struct runtime_task *task, unsigned int id);
extern void runtime_control_init(struct runtime_control *control);
extern void runtime_init();

#endif

