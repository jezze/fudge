struct kernel_module
{

    struct vfs_backend base;
    void *address;
    unsigned int limit;

};

void kernel_block_task();
void kernel_unblock_task();
void kernel_setup_modules(struct container *container, unsigned int count, struct kernel_module *modules);
struct container *kernel_setup();
