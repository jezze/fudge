#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

struct runtime_task
{

    struct mmu_header *header;
    struct vfs_descriptor descriptors[16];
    unsigned int (*add_descriptor)(struct runtime_task *task, struct vfs_node *node);
    struct vfs_descriptor *(*get_descriptor)(struct runtime_task *task, unsigned int index);
    void (*remove_descriptor)(struct runtime_task *task, unsigned int index);

};

extern struct runtime_task *runtime_get_running_task();
extern void runtime_init();

#endif

