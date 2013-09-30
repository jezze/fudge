void multi_activate(struct container *self, struct task *current);
void multi_map(struct container *self, unsigned int address);
struct task *multi_schedule(struct container *self);
struct task *multi_setup(struct container *container);
