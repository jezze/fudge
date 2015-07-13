struct task *scheduler_findactive();
struct task *scheduler_findinactive();
void scheduler_setstatus(struct task *task, unsigned int state);
unsigned int scheduler_rmessage(struct task *task, unsigned int size, unsigned int count, void *buffer);
unsigned int scheduler_wmessage(struct task *task, unsigned int size, unsigned int count, void *buffer);
void scheduler_registertask(struct task *task);
void scheduler_unregistertask(struct task *task);
void scheduler_setup();
