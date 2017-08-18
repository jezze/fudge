#define KERNEL_TASKS                    128
#define KERNEL_SERVERS                  32
#define KERNEL_MOUNTS                   32
#define KERNEL_SERVICES                 4096

struct task *kernel_getactivetask(void);
struct task *kernel_getinactivetask(void);
struct service_server *kernel_getfreeserver(void);
struct service_mount *kernel_getfreemount(void);
void kernel_walkmountparent(struct service *service);
void kernel_walkmountchild(struct service *service);
struct service *kernel_getservice(struct task *task, unsigned int service);
void kernel_activatetask(struct task *task);
void kernel_inactivatetask(struct task *task);
void kernel_blocktask(struct task *task);
void kernel_unblocktask(struct task *task);
void kernel_useserver(struct service_server *server);
void kernel_unuseserver(struct service_server *server);
void kernel_usemount(struct service_mount *mount);
void kernel_unusemount(struct service_mount *mount);
void kernel_copyservices(struct task *source, struct task *target);
unsigned int kernel_readtask(struct task *task, void *buffer, unsigned int count);
unsigned int kernel_writetask(struct task *task, void *buffer, unsigned int count);
void kernel_multicast(struct list *states, void *buffer, unsigned int count);
unsigned int kernel_setupbinary(struct task *task, unsigned int sp);
void kernel_setupramdisk(struct task *task, struct service_backend *backend);
struct task *kernel_setuptasks(void);
void kernel_setupservers(void);
void kernel_setupmounts(void);
void kernel_setupservices(void);
