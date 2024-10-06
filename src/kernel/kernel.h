#define KERNEL_TASKS                    64
#define KERNEL_LINKS                    128
#define KERNEL_MAILBOXES                512
#define KERNEL_CHANNELS                 4096

struct core *kernel_getcore(void);
void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct list_item *item));
unsigned int kernel_schedule(struct core *core);
unsigned int kernel_codebase(unsigned int itask, unsigned int address);
unsigned int kernel_loadprogram(unsigned int itask);
void kernel_signal(unsigned int itask, unsigned int signal);
struct task_thread *kernel_getthread(unsigned int itask);
unsigned int kernel_pick(unsigned int itask, unsigned int ichannel, struct message *message, unsigned int count, void *data);
unsigned int kernel_place(struct node *source, unsigned int ichannel, unsigned int event, unsigned int count, void *data);
unsigned int kernel_placetask(unsigned int itask, unsigned int ichannel, unsigned int event, unsigned int count, void *data);
unsigned int kernel_find(unsigned int itask, unsigned int count, char *name);
unsigned int kernel_announce(struct node *target);
void kernel_unannounce(struct node *node);
void kernel_unannouncetask(unsigned int itask);
void kernel_notify(struct node *node, unsigned int event, unsigned int count, void *data);
unsigned int kernel_createtask(void);
unsigned int kernel_loadtask(unsigned int itask, unsigned int ntask, unsigned int ip, unsigned int sp, unsigned int address);
void kernel_setup(unsigned int saddress, unsigned int ssize, unsigned int mbaddress, unsigned int mbsize);
