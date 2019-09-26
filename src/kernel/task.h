#define TASK_STATUS_NORMAL              0
#define TASK_STATUS_BLOCKED             1

struct task_thread
{

    unsigned int ip;
    unsigned int sp;
    unsigned int status;

};

struct task
{

    unsigned int id;
    struct resource resource;
    struct list_item item;
    struct task_thread thread;
    struct binary_format *format;
    struct binary_node node;

};

void task_initthread(struct task_thread *thread);
void task_init(struct task *task, unsigned int id);
