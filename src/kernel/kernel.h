#define KERNEL_VSTACK                   0x80000000
#define KERNEL_VMMAP                    0xC0000000
#define KERNEL_VMAILBOX                 0xA0000000

struct core *kernel_getcore(void);
unsigned int kernel_getchannelinode(unsigned int itask, unsigned int ichannel);
unsigned int kernel_linknode(unsigned int target, unsigned int source);
unsigned int kernel_unlinknode(unsigned int target, unsigned int source);
void kernel_schedule(struct core *core);
void kernel_signal(unsigned int itask, unsigned int signal);
unsigned int kernel_pick(unsigned int source, struct message *message, unsigned int count, void *data);
unsigned int kernel_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data);
unsigned int kernel_announce(unsigned int inode, unsigned int namehash);
void kernel_notify(unsigned int source, unsigned int event, unsigned int count, void *data);
unsigned int kernel_loadtask(unsigned int itask, unsigned int ip, unsigned int sp, unsigned int address);
void kernel_setcallback(struct core *(*getcore)(void), void (*assigncore)(unsigned int itask));
void kernel_setup(void);
