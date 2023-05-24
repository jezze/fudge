#define KERNEL_TASKS                    64
#define KERNEL_LINKS                    128
#define KERNEL_DESCRIPTORS              32

struct core *kernel_getcore(void);
void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct list_item *item));
void kernel_addlink(struct list *list, unsigned int target, unsigned int source);
void kernel_removelink(struct list *list, unsigned int target);
struct task *kernel_schedule(struct core *core);
struct descriptor *kernel_getdescriptor(struct task *task, unsigned int descriptor);
void kernel_kill(unsigned int source, unsigned int target);
unsigned int kernel_pick(unsigned int source, struct message *message, void *data);
unsigned int kernel_place(unsigned int source, unsigned int target, struct message *message, void *data);
void kernel_notify(struct list *states, unsigned int event, void *buffer, unsigned int count);
struct task *kernel_createtask(void);
void kernel_setuptask(struct task *task, unsigned int sp);
void kernel_setup(unsigned int mbaddress, unsigned int mbsize);
