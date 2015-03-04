unsigned int kernel_call(unsigned int index, struct container *container, struct task *task, void *stack);
void kernel_copyprogram(struct task *task);
void kernel_setuptask(struct task *task, unsigned int sp);
unsigned int kernel_setupramdisk(struct container *container, struct task *task, struct vfs_backend *backend);
void kernel_setup(unsigned int (*spawn)(struct container *self, struct task *task, void *stack), unsigned int (*despawn)(struct container *self, struct task *task, void *stack));
