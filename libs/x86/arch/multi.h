void multi_map(struct container *self, unsigned int address);
struct task *multi_schedule(struct container *self, struct cpu_general *general, struct cpu_interrupt *interrupt);
struct task *multi_setup(struct container *container);
