unsigned int abi_call(unsigned int index, unsigned int itask, void *stack);
void abi_setcallback(unsigned int index, unsigned int (*callback)(unsigned int itask, void *stack));
void abi_setup(void);
