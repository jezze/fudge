#define TASK_DESCRIPTORS                32
#define TASK_MAILBOXSIZE                4096

struct task_state
{

    struct list_item item;
    struct {unsigned long ip; unsigned long sp;} registers;
    unsigned int blocked;

};

struct task_mailbox
{

    struct list_item item;
    struct buffer buffer;
    unsigned char data[TASK_MAILBOXSIZE];

};

struct task
{

    struct resource resource;
    struct task_state state;
    struct task_mailbox mailbox;
    struct vfs_descriptor descriptors[TASK_DESCRIPTORS];

};

void task_copy(struct task *task, struct task *next);
void task_init(struct task *task);
