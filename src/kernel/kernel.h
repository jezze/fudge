#define KERNEL_TASKS                    16
#define KERNEL_LINKS                    128
#define KERNEL_DESCRIPTORS              32
#define KERNEL_CHANNELS                 0x4000

struct core *kernel_getcore(void);
void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct list_item *item));
void kernel_addlink(struct list *list, unsigned int target, unsigned int source);
void kernel_removelink(struct list *list, unsigned int target);
unsigned int kernel_schedule(struct core *core);
unsigned int kernel_codebase(unsigned int itask, unsigned int address);
unsigned int kernel_loadprogram(unsigned int itask);
void kernel_signal(unsigned int itask, unsigned int signal);
struct task_thread *kernel_getthread(unsigned int itask);
struct descriptor *kernel_getdescriptor(unsigned int itask, unsigned int descriptor);
void kernel_setdescriptor(unsigned int itask, unsigned int descriptor, struct service *service, unsigned int id);
void kernel_copydescriptor(unsigned int itask, unsigned int descriptor, unsigned int ptask, unsigned int pdescriptor);
unsigned int kernel_pick(unsigned int source, struct message *message, void *data);
unsigned int kernel_place(unsigned int source, unsigned int ichannel, unsigned int event, unsigned int count, void *data);
void kernel_announce(unsigned int itask, unsigned int ichannel);
void kernel_notify(struct list *states, unsigned int event, unsigned int count, void *data);
unsigned int kernel_createtask(void);
unsigned int kernel_loadtask(unsigned int itask, unsigned int sp, unsigned int descriptor);
void kernel_setup(unsigned int saddress, unsigned int ssize, unsigned int mbaddress, unsigned int mbsize);
