void kernel_copydescriptors(struct container *container, struct task *source, struct task *target);
void kernel_multicast(struct list *links, void *buffer, unsigned int count);
unsigned int kernel_setupbinary(struct container *container, struct task *task, unsigned int sp);
void kernel_setupramdisk(struct container *container, struct task *task, struct service_backend *backend);
void kernel_setup(void);
