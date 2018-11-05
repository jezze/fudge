#define TASK_MAILBOXSIZE                4096
#define TASK_STATUS_NORMAL              0
#define TASK_STATUS_BLOCKED             1

struct task_thread
{

    unsigned int ip;
    unsigned int sp;
    unsigned int status;

};

struct task_mailbox
{

    struct ring ring;
    struct spinlock spinlock;

};

struct task
{

    unsigned int id;
    struct resource resource;
    struct list_item item;
    struct task_thread thread;
    struct task_mailbox mailbox;
    struct binary_format *format;
    struct binary_node node;

};

unsigned int task_readall(struct task *task, void *buffer, unsigned int count);
unsigned int task_writeall(struct task *task, void *buffer, unsigned int count);
void task_initthread(struct task_thread *thread);
void task_initmailbox(struct task_mailbox *mailbox, char *buffer);
void task_init(struct task *task, unsigned int id, char *buffer);
