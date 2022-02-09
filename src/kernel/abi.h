unsigned int abi_call(unsigned int index, struct task *task, void *stack);
void abi_setcallback(unsigned int index, unsigned int (*callback)(struct task *task, void *stack));
void abi_setup(void);
