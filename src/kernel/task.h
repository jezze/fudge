#define TASK_DESCRIPTORS                32
#define TASK_MAILBOXSIZE                4096
#define TASK_STATUS_INACTIVE            0
#define TASK_STATUS_ACTIVE              1
#define TASK_STATUS_UNBLOCKED           2
#define TASK_STATUS_BLOCKED             3

struct task_mailbox
{

    struct task *task;
    struct list_item item[TASK_DESCRIPTORS];
    struct buffer buffer;
    unsigned char data[TASK_MAILBOXSIZE];

};

struct task_state
{

    struct list_item item;
    struct {unsigned long ip; unsigned long sp;} registers;
    unsigned int status;

};

struct task
{

    struct resource resource;
    struct task_state state;
    struct task_mailbox mailbox;
    unsigned int id;

};

struct task *task_findactive(void);
struct task *task_findinactive(void);
struct list_item *task_getmailbox(struct task *task, unsigned int descriptor);
void task_setstatus(struct task *task, unsigned int state);
void task_resume(struct task *task, unsigned int ip, unsigned int sp);
void task_register(struct task *task);
void task_unregister(struct task *task);
void task_initmailbox(struct task_mailbox *mailbox, struct task *task);
void task_init(struct task *task, unsigned int id);
