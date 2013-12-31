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
    struct {unsigned long ip; unsigned long sp;} registers;
    struct task_descriptor descriptors[TASK_DESCRIPTORS];

};

void task_init(struct task *task, enum task_state state, unsigned long ip, unsigned long sp);
