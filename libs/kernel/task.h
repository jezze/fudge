#define TASK_DESCRIPTORS                16
#define TASKADDRESS_PHYSICAL            0x00400000
#define TASKADDRESS_VIRTUAL             0x08048000
#define TASKADDRESS_SIZE                0x00010000
#define STACKADDRESS_PHYSICAL           0x00810000
#define STACKADDRESS_VIRTUAL            0x80000000
#define STACKADDRESS_SIZE               0x00010000

enum task_state
{

    TASK_STATE_USED                     = 1,
    TASK_STATE_IDLE                     = 2

};

struct task_descriptor
{

    struct vfs_session session;
    unsigned int id;

};

struct task
{

    enum task_state state;
    struct {unsigned int ip; unsigned int sp; unsigned int fp; unsigned int status;} registers;
    struct task_descriptor descriptors[TASK_DESCRIPTORS];

};

void task_init(struct task *task, unsigned int ip, unsigned int sp, unsigned int fp);
