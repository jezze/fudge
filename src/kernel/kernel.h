void kernel_copytask(struct task *task, struct task *next);
void kernel_copyprogram(struct task *task);
void kernel_setuptask(struct task *task, unsigned int sp);
unsigned int kernel_setupramdisk(struct container *container, struct task *task, struct vfs_backend *backend);
void kernel_setup();
