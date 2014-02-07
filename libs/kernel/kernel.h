struct kernel_module
{

    struct vfs_backend base;
    void *address;
    unsigned int limit;

};

void kernel_setup_modules(struct container *container, struct task *task, unsigned int count, struct kernel_module *modules);
void kernel_setup();
