#define KERNEL_TASKS                    128
#define KERNEL_LINKS                    128
#define KERNEL_DESCRIPTORS              32
#define KERNEL_MAILBOXES                128

unsigned int kernel_walk(struct service_descriptor *descriptor, char *path, unsigned int length);
struct core *kernel_getcore(void);
void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct task *task));
struct link *kernel_picklink(unsigned int source);
void kernel_freelink(struct link *link);
struct task *kernel_picktask(void);
void kernel_freetask(unsigned int id);
void kernel_readytask(unsigned int id);
void kernel_killtask(unsigned int id);
void kernel_schedule(struct core *core);
struct service_descriptor *kernel_getdescriptor(struct task *task, unsigned int descriptor);
void kernel_copydescriptors(struct task *source, struct task *target);
void kernel_reset(unsigned int id);
unsigned int kernel_pick(unsigned int source, struct message_header *header, void *data);
unsigned int kernel_place(unsigned int source, unsigned int target, struct message_header *header, void *data);
void kernel_notify(struct list *states, unsigned int type, void *buffer, unsigned int count);
unsigned int kernel_setupbinary(struct task *task, unsigned int sp);
void kernel_setupinit(struct task *task);
void kernel_setup(unsigned int mbaddress, unsigned int mbsize);
