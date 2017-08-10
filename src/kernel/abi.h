unsigned int abi_call(unsigned int index, struct task *task, void *stack);
void abi_setup(unsigned int (*spawn)(struct task *task, void *stack), unsigned int (*despawn)(struct task *task, void *stack));
