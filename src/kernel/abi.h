unsigned int abi_call(unsigned int index, struct container *container, struct task *task, void *stack);
void abi_setup(unsigned int (*spawn)(struct container *container, struct task *task, void *stack), unsigned int (*despawn)(struct container *container, struct task *task, void *stack));
