struct task *scheduler_findactive();
struct task *scheduler_findinactive();
void scheduler_setstatus(struct task *task, unsigned int state);
void scheduler_registertask(struct task *task);
void scheduler_unregistertask(struct task *task);
void scheduler_setup();
