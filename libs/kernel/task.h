#define TASK_DESCRIPTORS                32

enum task_state
{

    TASK_STATE_USED                     = 1,
    TASK_STATE_IDLE                     = 2,
    TASK_STATE_BLOCKED                  = 4

};

struct task_descriptor
{

    struct vfs_session session;
    unsigned int id;

};

struct task
{

    enum task_state state;
    struct list_item item;
    struct {unsigned long ip; unsigned long sp;} registers;
    struct task_descriptor descriptors[TASK_DESCRIPTORS];

};

struct task *task_sched_find_next_task();
struct task *task_sched_find_free_task();
void task_sched_add(struct task *task);
void task_sched_init();
void task_set_flag(struct task *task, enum task_state state);
void task_unset_flag(struct task *task, enum task_state state);
unsigned int task_test_flag(struct task *task, enum task_state state);
void task_init(struct task *task, enum task_state state, unsigned long ip, unsigned long sp);
