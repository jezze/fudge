#define KERNEL_CODESIZE                 0x500000
#define KERNEL_STACKSIZE                0x2000
#define KERNEL_VSTACK                   0x80000000
#define KERNEL_VMAILBOX                 0x40000000
#define KERNEL_VMEM                     0x50000000
#define KERNEL_VBUFFER                  0x70000000
#define KERNEL_VMMAP                    0xC0000000

struct core *kernel_getcore(void);
unsigned int kernel_getchannelinode(unsigned int itask, unsigned int ichannel);
unsigned int kernel_linknode(struct resource *resource, unsigned int source);
unsigned int kernel_unlinknode(struct resource *resource, unsigned int source);
void kernel_schedule(struct core *core);
unsigned int kernel_kill(unsigned int itask);
void kernel_signal(unsigned int itask, unsigned int signal);
unsigned int kernel_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data);
unsigned int kernel_announce(unsigned int inode, char *name);
void kernel_notify(struct resource *resource, unsigned int event, unsigned int count, void *data);
void kernel_starttask(unsigned int itask);
unsigned int kernel_loadtask(unsigned int itask, unsigned long base, unsigned int id, unsigned long mmap, unsigned long code, unsigned long stack, unsigned long buffer);
unsigned long kernel_getbuffer(unsigned int inode);
void kernel_setcallback(struct core *(*getcore)(void), void (*assigncore)(unsigned int itask));
void kernel_setup(void);
