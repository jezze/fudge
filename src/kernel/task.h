#define TASK_DESCRIPTORS                32
#define TASK_MAILBOXSIZE                4096
#define TASK_STATUS_INACTIVE            0
#define TASK_STATUS_ACTIVE              1
#define TASK_STATUS_UNBLOCKED           2
#define TASK_STATUS_BLOCKED             3

struct task_mailbox
{

    struct ring ring;
    unsigned char buffer[TASK_MAILBOXSIZE];

};

struct task_state
{

    struct list_item item;
    struct {unsigned long ip; unsigned long sp;} registers;
    unsigned int status;
    unsigned int rewind;

};

struct task_descriptor
{

    struct list_item link;
    struct task *task;
    struct container_server *server;
    unsigned int id;
    unsigned int offset;
    unsigned int current;

};

struct task
{

    struct resource resource;
    struct task_state state;
    struct task_mailbox mailbox;
    struct binary_format *format;
    struct binary_node node;
    struct task_descriptor descriptors[TASK_DESCRIPTORS];

};

struct task *task_findactive(void);
struct task *task_findinactive(void);
void task_setstatus(struct task *task, unsigned int state);
void task_resume(struct task *task, unsigned int ip, unsigned int sp);
unsigned int task_read(struct task *task, void *buffer, unsigned int count);
unsigned int task_write(struct task *task, void *buffer, unsigned int count);
void task_register(struct task *task);
void task_unregister(struct task *task);
void task_initdescriptor(struct task_descriptor *descriptor, struct task *task);
void task_init(struct task *task);
