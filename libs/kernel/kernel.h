struct kernel_module
{

    struct vfs_backend base;
    void *address;
    unsigned int limit;

};

unsigned int kernel_call(unsigned int index, struct container *container, struct task *task, void *stack);
void kernel_setupmodules(struct container *container, struct task *task, unsigned int count, struct kernel_module *modules);
void kernel_setup(unsigned int (*spawn)(struct container *self, struct task *task, void *stack), unsigned int (*despawn)(struct container *self, struct task *task, void *stack));
