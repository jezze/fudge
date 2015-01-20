#define TASK_DESCRIPTORS                32
#define TASK_MAILBOXSIZE                4096

struct task_mailbox
{

    struct list_item item;
    struct buffer buffer;
    unsigned char data[TASK_MAILBOXSIZE];
    void *owner;

};

struct task
{

    struct list_item item;
    struct resource resource;
    struct task_mailbox mailbox;
    struct {unsigned long ip; unsigned long sp;} registers;
    unsigned int blocked;
    struct vfs_descriptor descriptors[TASK_DESCRIPTORS];

};

void task_initmailbox(struct task_mailbox *mailbox, void *owner);
void task_init(struct task *task, unsigned long ip, unsigned long sp);
