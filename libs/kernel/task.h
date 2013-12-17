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
    struct {unsigned int ip; unsigned int sp; unsigned int fp;} registers;
    struct task_descriptor descriptors[TASK_DESCRIPTORS];

};

void task_init(struct task *task, unsigned int ip, unsigned int sp, unsigned int fp);
