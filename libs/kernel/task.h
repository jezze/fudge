#define TASK_DESCRIPTORS                32

struct task_descriptor
{

    struct vfs_session session;
    unsigned int id;

};

struct task
{

    struct list_item item;
    struct {unsigned long ip; unsigned long sp;} registers;
    struct task_descriptor descriptors[TASK_DESCRIPTORS];

};

struct task *task_sched_find_next_task();
struct task *task_sched_find_free_task();
void task_sched_block(struct task *task);
void task_sched_unblock(struct task *task);
void task_sched_use(struct task *task);
void task_sched_unuse(struct task *task);
void task_sched_add(struct task *task);
void task_sched_init();
void task_init(struct task *task, unsigned long ip, unsigned long sp);
