#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

struct runtime_task
{

    unsigned int pid;
    unsigned int parentpid;
    unsigned int running;
    unsigned int used;
    void *eip;
    void *esp;
    void *ebp;
    void (*create_stack)(struct runtime_task *self, void *address, void *virt, unsigned int argc, char **argv);
    unsigned int (*load)(struct runtime_task *self, char *path, unsigned int argc, char **argv);
    void (*unload)(struct runtime_task *self);
    void (*save)(struct runtime_task *self, void *eip, void *esp, void *ebp);
    struct mmu_header *header;
    struct vfs_descriptor descriptors[16];
    struct vfs_descriptor *(*add_descriptor)(struct runtime_task *self, struct vfs_node *node);
    struct vfs_descriptor *(*get_descriptor)(struct runtime_task *self, unsigned int index);
    void (*remove_descriptor)(struct runtime_task *self, unsigned int index);

};

extern struct runtime_task *runtime_get_task(unsigned int pid);
extern struct runtime_task *runtime_get_running_task();
extern struct runtime_task *runtime_get_free_task();
extern void runtime_activate(struct runtime_task *task);
extern void runtime_relocate(void *paddress);
extern void runtime_task_init(struct runtime_task *task, unsigned int pid);
extern void runtime_init();

#endif

