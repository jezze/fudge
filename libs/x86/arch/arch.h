void arch_halt();
void arch_usermode(unsigned int code, unsigned int data, unsigned int ip, unsigned int sp);
void arch_isrgeneralfault();
void arch_isrpagefault();
void arch_isrsyscall();
unsigned short arch_segment();
unsigned short arch_schedule(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_setup(unsigned int count, struct kernel_module *modules);
