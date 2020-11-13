#define TASK_STATUS_NORMAL              0
#define TASK_STATUS_BLOCKED             1

struct task_thread
{

    unsigned int ip;
    unsigned int sp;

};

struct task
{

    struct resource resource;
    struct list_item item;
    struct task_thread thread;
    struct binary_node node;
    struct binary_format *format;
    unsigned int id;

};

void task_register(struct task *task);
void task_unregister(struct task *task);
void task_initthread(struct task_thread *thread);
void task_init(struct task *task, unsigned int id);
