void multi_map(unsigned int address);
struct task *multi_schedule(struct task *task, struct cpu_general *general, struct cpu_interrupt *interrupt);
struct task *multi_setup(struct container *container);
