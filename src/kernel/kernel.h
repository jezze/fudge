#define KERNEL_CONTAINERS               16
#define KERNEL_TASKS                    128
#define KERNEL_SERVICES                 4096

struct task *kernel_findactivetask(void);
struct task *kernel_findinactivetask(void);
void kernel_inactivatetask(struct task *task);
void kernel_activatetask(struct task *task);
void kernel_blocktask(struct task *task);
void kernel_unblocktask(struct task *task);
struct service *kernel_getservice(struct task *task, unsigned int service);
void kernel_copyservices(struct task *source, struct task *target);
unsigned int kernel_readtask(struct task *task, void *buffer, unsigned int count);
unsigned int kernel_writetask(struct task *task, void *buffer, unsigned int count);
void kernel_multicast(struct list *links, void *buffer, unsigned int count);
unsigned int kernel_setupbinary(struct task *task, unsigned int sp);
void kernel_setupramdisk(struct container *container, struct task *task, struct service_backend *backend);
struct container *kernel_setupcontainers(void);
struct task *kernel_setuptasks(void);
void kernel_setupservices(void);
void kernel_setup(void);
