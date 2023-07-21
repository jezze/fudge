#define TASK_STATE_DEAD                 0
#define TASK_STATE_NEW                  1
#define TASK_STATE_BLOCKED              2
#define TASK_STATE_UNBLOCKED            3
#define TASK_STATE_ASSIGNED             4
#define TASK_STATE_RUNNING              5
#define TASK_SIGNAL_KILL                1
#define TASK_SIGNAL_BLOCK               2
#define TASK_SIGNAL_UNBLOCK             3

struct task_signals
{

    unsigned int kills;
    unsigned int blocks;
    unsigned int unblocks;

};

struct task_thread
{

    unsigned int ip;
    unsigned int sp;

};

struct task
{

    struct resource resource;
    struct task_signals signals;
    struct task_thread thread;
    struct binary_node node;
    struct spinlock spinlock;
    unsigned int id;
    unsigned int state;

};

void task_signal(struct task *task, unsigned int signal);
unsigned int task_transition(struct task *task, unsigned int state);
void task_register(struct task *task);
void task_unregister(struct task *task);
void task_resetsignals(struct task_signals *signals);
void task_resetthread(struct task_thread *thread);
void task_initsignals(struct task_signals *signals);
void task_initthread(struct task_thread *thread);
void task_reset(struct task *task);
void task_init(struct task *task, unsigned int id);
