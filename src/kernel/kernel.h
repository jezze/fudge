#define KERNEL_TASKS                    64
#define KERNEL_LINKS                    128
#define KERNEL_DESCRIPTORS              32

struct core *kernel_getcore(void);
void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct list_item *item));
void kernel_addlink(struct list *list, unsigned int target, unsigned int source);
void kernel_removelink(struct list *list, unsigned int target);
struct task *kernel_schedule(struct core *core);
struct descriptor *kernel_getdescriptor(unsigned int task, unsigned int descriptor);
void kernel_setdescriptor(unsigned int task, unsigned int descriptor, struct service *service, unsigned int id);
void kernel_copydescriptor(unsigned int task, unsigned int descriptor, unsigned int ptask, unsigned int pdescriptor);
void kernel_kill(unsigned int source, unsigned int target);
unsigned int kernel_pick(unsigned int source, struct message *message, void *data);
unsigned int kernel_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data);
void kernel_notify(struct list *states, unsigned int event, unsigned int count, void *data);
unsigned int kernel_createtask(void);
void kernel_setuptask(unsigned int task, unsigned int sp, unsigned int descriptor);
void kernel_setup(unsigned int mbaddress, unsigned int mbsize);
