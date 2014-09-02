struct scheduler_rendezvous
{

    struct task *task;

};

unsigned int scheduler_rendezvous_asleep(struct scheduler_rendezvous *rendezvous);
void scheduler_rendezvous_sleep(struct scheduler_rendezvous *rendezvous, unsigned int condition);
void scheduler_rendezvous_unsleep(struct scheduler_rendezvous *rendezvous);
struct task *scheduler_findusedtask();
struct task *scheduler_findfreetask();
void scheduler_block(struct task *task);
void scheduler_unblock(struct task *task);
void scheduler_use(struct task *task);
void scheduler_unuse(struct task *task);
void scheduler_register_task(struct task *task);
void scheduler_unregister_task(struct task *task);
void scheduler_setup();
