#define KERNEL_TASKS                    128
#define KERNEL_SERVERS                  32
#define KERNEL_MOUNTS                   32
#define KERNEL_SERVICES                 4096

struct task *kernel_getfreetask(void);
struct service_server *kernel_pickserver(void);
struct service_mount *kernel_pickmount(void);
void kernel_walkmountparent(struct service *service);
void kernel_walkmountchild(struct service *service);
struct service *kernel_getservice(struct task *task, unsigned int service);
void kernel_freetask(struct task *task);
void kernel_readytask(struct task *task);
void kernel_blocktask(struct task *task);
void kernel_unblocktask(struct task *task);
struct task *kernel_schedule(struct core *core, void (*assign)(struct task *task));
void kernel_useserver(struct service_server *server);
void kernel_freeserver(struct service_server *server);
void kernel_usemount(struct service_mount *mount);
void kernel_freemount(struct service_mount *mount);
void kernel_copyservices(struct task *source, struct task *target);
unsigned int kernel_readmailbox(struct task *task, void *buffer, unsigned int count);
unsigned int kernel_writemailbox(struct task *task, void *buffer, unsigned int count);
void kernel_unblockall(struct list *states);
void kernel_multicast(struct list *states, void *buffer, unsigned int count);
unsigned int kernel_setupbinary(struct task *task, unsigned int sp);
void kernel_setupramdisk(struct task *task, struct service_backend *backend);
void kernel_setuptasks(void);
void kernel_setupservers(void);
void kernel_setupmounts(void);
void kernel_setupservices(void);
