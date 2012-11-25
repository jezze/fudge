void syscall_routine();
void syscall_begin(struct isr_registers *registers);
unsigned short syscall_complete(struct isr_registers *registers);
void syscall_setup_arch(struct runtime_task *task, unsigned short selector);
