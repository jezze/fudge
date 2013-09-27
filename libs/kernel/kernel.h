struct kernel_module
{

    struct vfs_backend base;
    void *address;
    unsigned int limit;

};

enum kernel_lock_state
{

    LOCK_STATE_UNLOCKED                 = 0,
    LOCK_STATE_LOCKED                   = 1

};

struct kernel_lock
{

    enum kernel_lock_state state;

};

struct kernel_rendezvous
{

    struct task *task;

};

void kernel_rendezvous_sleep(struct kernel_rendezvous *rendezvous, unsigned int condition);
void kernel_rendezvous_unsleep(struct kernel_rendezvous *rendezvous, unsigned int condition);
void kernel_setup_modules(unsigned int count, struct kernel_module *modules);
struct container *kernel_setup();
