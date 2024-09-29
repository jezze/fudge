#define KERNEL_TASKS                    64
#define KERNEL_LINKS                    64
#define KERNEL_MAILBOXES                64
#define KERNEL_DESCRIPTORS              32
#define KERNEL_CHANNELS                 4096

struct core *kernel_getcore(void);
void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct list_item *item));
unsigned int kernel_link(unsigned int ichannel, unsigned int target);
unsigned int kernel_unlink(unsigned int ichannel, unsigned int target);
unsigned int kernel_schedule(struct core *core);
unsigned int kernel_codebase(unsigned int itask, unsigned int address);
unsigned int kernel_loadprogram(unsigned int itask);
void kernel_signal(unsigned int itask, unsigned int signal);
struct task_thread *kernel_getthread(unsigned int itask);
unsigned int kernel_pick(unsigned int itask, struct message *message, unsigned int count, void *data);
unsigned int kernel_place(unsigned int source, unsigned int ichannel, unsigned int event, unsigned int count, void *data);
unsigned int kernel_place2(unsigned int itask, unsigned int ichannel, unsigned int event, unsigned int count, void *data);
unsigned int kernel_find(unsigned int source, unsigned int count, char *name);
unsigned int kernel_announce(void *interface, unsigned int (*place)(void *interface, unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data));
void kernel_unannounce(unsigned int ichannel);
void kernel_unannouncetask(unsigned int itask);
void kernel_notify(unsigned int ichannel, unsigned int event, unsigned int count, void *data);
unsigned int kernel_createtask(void);
unsigned int kernel_loadtask(unsigned int itask, unsigned int ip, unsigned int sp, unsigned int address);
void kernel_setup(unsigned int saddress, unsigned int ssize, unsigned int mbaddress, unsigned int mbsize);
