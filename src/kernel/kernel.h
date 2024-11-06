#define KERNEL_TASKS                    64
#define KERNEL_NODES                    1024
#define KERNEL_MAILBOXES                512

struct node *kernel_getnode(struct list *list, unsigned int index);
struct core *kernel_getcore(void);
struct node *kernel_link(struct list *nodes, struct mailbox *mailbox, struct resource *resource, unsigned int (*place)(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data));
void kernel_unlink(struct list *targets, struct node *target);
void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct list_item *item));
unsigned int kernel_schedule(struct core *core);
unsigned int kernel_codebase(unsigned int itask, unsigned int address);
unsigned int kernel_loadprogram(unsigned int itask);
void kernel_signal(unsigned int itask, unsigned int signal);
struct task_thread *kernel_getthread(unsigned int itask);
unsigned int kernel_pick(unsigned int itask, unsigned int index, struct message *message, unsigned int count, void *data);
unsigned int kernel_place(unsigned int index, struct list *sources, struct node *target, unsigned int event, unsigned int count, void *data);
unsigned int kernel_placetask(unsigned int itask, unsigned int index, unsigned int target, unsigned int event, unsigned int count, void *data);
unsigned int kernel_find(unsigned int itask, unsigned int count, char *name);
void kernel_notify(unsigned int index, struct list *sources, struct list *targets, unsigned int event, unsigned int count, void *data);
unsigned int kernel_createtask(void);
unsigned int kernel_loadtask(unsigned int itask, unsigned int ntask, unsigned int ip, unsigned int sp, unsigned int address);
void kernel_setup(unsigned int saddress, unsigned int ssize, unsigned int mbaddress, unsigned int mbsize);
