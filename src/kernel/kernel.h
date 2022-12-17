#define KERNEL_TASKS                    64
#define KERNEL_LINKS                    128
#define KERNEL_DESCRIPTORS              32
#define KERNEL_MAILBOXES                128

struct core *kernel_getcore(void);
void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct list_item *item));
void kernel_addlink(unsigned int source, struct list *list);
void kernel_removelink(unsigned int source, struct list *list);
struct task *kernel_schedule(struct core *core, struct task *coretask);
struct descriptor *kernel_getdescriptor(struct task *task, unsigned int descriptor);
void kernel_kill(unsigned int source, unsigned int target);
unsigned int kernel_pick(unsigned int source, struct message_header *header, void *data);
unsigned int kernel_place(unsigned int source, unsigned int target, struct message_header *header, void *data);
void kernel_notify(struct list *states, unsigned int type, void *buffer, unsigned int count);
struct task *kernel_loadtask(struct task *parent, unsigned int sp);
void kernel_setup(unsigned int mbaddress, unsigned int mbsize);
