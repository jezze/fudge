#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

#define RUNTIME_TASK_SLOTS 8
#define RUNTIME_TASK_DESCRIPTOR_SLOTS 16

struct runtime_registers
{

    void *ip;
    void *sp;
    void *sb;

};

struct runtime_task
{

    unsigned int id;
    unsigned int parentid;
    unsigned int running;
    unsigned int used;
    struct runtime_registers registers;
    void (*save_registers)(struct runtime_task *self, void *ip, void *sp, void *sb);
    void (*create_stack)(struct runtime_task *self, void *address, void *virt, unsigned int argc, char **argv);
    unsigned int (*load)(struct runtime_task *self, char *path, unsigned int argc, char **argv);
    void (*unload)(struct runtime_task *self);
    struct mmu_header *header;
    struct vfs_descriptor descriptors[RUNTIME_TASK_DESCRIPTOR_SLOTS];
    struct vfs_descriptor *(*add_descriptor)(struct runtime_task *self, struct vfs_node *node);
    struct vfs_descriptor *(*get_descriptor)(struct runtime_task *self, unsigned int index);
    void (*remove_descriptor)(struct runtime_task *self, unsigned int index);

};

extern struct runtime_task *runtime_get_task(unsigned int id);
extern struct runtime_task *runtime_get_running_task();
extern struct runtime_task *runtime_get_slot();
extern void runtime_activate(struct runtime_task *task);
extern void *runtime_relocate(void *paddress);
extern void runtime_task_init(struct runtime_task *task, unsigned int id);
extern void runtime_init();

#endif

