#define TASK_DESCRIPTORS                32
#define TASK_MAILBOXSIZE                4096
#define TASK_STATUS_INACTIVE            0
#define TASK_STATUS_ACTIVE              1
#define TASK_STATUS_UNBLOCKED           2
#define TASK_STATUS_BLOCKED             3

struct task_state
{

    struct list_item item;
    unsigned int ip;
    unsigned int sp;
    unsigned int status;
    unsigned int rewind;

};

struct task
{

    unsigned int id;
    struct resource resource;
    struct task_state state;
    struct binary_format *format;
    struct binary_node node;
    struct ring mailbox;
    unsigned char mailboxdata[TASK_MAILBOXSIZE];

};

void task_setstate(struct task *task, unsigned int ip, unsigned int sp);
void task_init(struct task *task, unsigned int id);
