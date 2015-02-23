enum
{

    KERNEL_WALK                         = 0x01,
    KERNEL_CREATE                       = 0x02,
    KERNEL_DESTROY                      = 0x03,
    KERNEL_OPEN                         = 0x04,
    KERNEL_CLOSE                        = 0x05,
    KERNEL_READ                         = 0x06,
    KERNEL_WRITE                        = 0x07,
    KERNEL_AUTH                         = 0x08,
    KERNEL_MOUNT                        = 0x09,
    KERNEL_BIND                         = 0x0A,
    KERNEL_LOAD                         = 0x0B,
    KERNEL_UNLOAD                       = 0x0C,
    KERNEL_SPAWN                        = 0x0D,
    KERNEL_DESPAWN                      = 0x0E

};

struct kernel_module
{

    struct vfs_backend base;
    void *address;
    unsigned int limit;

};

unsigned int kernel_call(unsigned int index, struct container *container, struct task *task, void *stack);
void kernel_setupmodules(struct container *container, struct task *task, unsigned int count, struct kernel_module *modules);
void kernel_setup(unsigned int (*spawn)(struct container *self, struct task *task, void *stack), unsigned int (*despawn)(struct container *self, struct task *task, void *stack));
