unsigned int kernel_call(unsigned int index, struct container *container, struct task *task, void *stack);
unsigned int kernel_rmessage(struct task *task, unsigned int size, unsigned int count, void *buffer);
unsigned int kernel_wmessage(struct task *task, unsigned int size, unsigned int count, void *buffer);
void kernel_copytask(struct task *task, struct task *next);
void kernel_copyprogram(struct task *task);
void kernel_setuptask(struct task *task, unsigned int sp);
unsigned int kernel_setupramdisk(struct container *container, struct task *task, struct vfs_backend *backend);
void kernel_setup(unsigned int (*spawn)(struct container *container, struct task *task, void *stack), unsigned int (*despawn)(struct container *container, struct task *task, void *stack));
