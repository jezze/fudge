#define KERNEL_TASKS                    128
#define KERNEL_DESCRIPTORS              32
#define KERNEL_MOUNTS                   32

void kernel_walk(struct service_descriptor *descriptor, char *path, unsigned int length);
struct core *kernel_getcore(void);
void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct task *task));
struct task *kernel_picktask(void);
struct service_mount *kernel_pickmount(void);
void kernel_usetask(struct task *task);
void kernel_usemount(struct service_mount *mount);
void kernel_freetask(struct task *task);
void kernel_freemount(struct service_mount *mount);
void kernel_blocktask(struct task *task);
void kernel_unblocktask(struct task *task);
void kernel_schedule(struct core *core);
struct service_descriptor *kernel_getdescriptor(struct task *task, unsigned int descriptor);
void kernel_copydescriptors(struct task *source, struct task *target);
void kernel_multicast(struct list *states, void *buffer, unsigned int count);
unsigned int kernel_setupbinary(struct task *task, unsigned int sp);
void kernel_setupinit(struct task *task);
void kernel_setup(void);
