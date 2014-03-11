struct task *scheduler_find_used_task();
struct task *scheduler_find_free_task();
void scheduler_block(struct task *task);
void scheduler_unblock(struct task *task);
void scheduler_use(struct task *task);
void scheduler_unuse(struct task *task);
void scheduler_register_task(struct task *task);
void scheduler_setup();
