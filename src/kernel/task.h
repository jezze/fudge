#define TASK_CODESIZE                   0x00200000
#define TASK_STACKSIZE                  0x00008000
#define TASK_RETRIESLOG                 0x10000
/* two tasks each retrying a send to the other this long are stuck: neither can make room for the other */
#define TASK_RETRIESDEADLOCK            0x1000
#define TASK_STATE_DEAD                 1
#define TASK_STATE_BLOCKED              2
#define TASK_STATE_ASSIGNED             3
#define TASK_STATE_RUNNING              4
#define TASK_SIGNAL_KILL                1
#define TASK_SIGNAL_BLOCK               2
#define TASK_SIGNAL_UNBLOCK             3
#define TASK_ABORT_PENDING              1
#define TASK_ABORT_DONE                 2

struct task_signals
{

    unsigned int kill;
    unsigned int block;
    unsigned int unblock;

};

struct task_thread
{

    unsigned long ip;
    unsigned long sp;

};

struct task
{

    struct task_signals signals;
    struct task_thread thread;
    struct task_thread start;
    struct spinlock spinlock;
    unsigned int state;
    unsigned long mmap;
    unsigned int retries;
    unsigned int sendingto;
    unsigned int aborted;
    unsigned int started;
    unsigned int imailbox[MESSAGE_CHANNELS];

};

void task_signal(struct task *task, unsigned int signal);
void task_register(struct task *task, unsigned long mmap, unsigned long ip, unsigned long sp);
void task_reset(struct task *task);
void task_init(struct task *task);
