#define TASK_DESCRIPTORS                32
#define TASK_RESOURCE                   3

struct task
{

    struct list_item item;
    struct resource resource;
    unsigned int sleep;
    struct {unsigned long ip; unsigned long sp;} registers;
    struct vfs_descriptor descriptors[TASK_DESCRIPTORS];

};

unsigned int task_sleep(struct task *task);
unsigned int task_unsleep(struct task *task);
void task_init(struct task *task, unsigned long ip, unsigned long sp);
