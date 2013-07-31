struct kernel_module
{

    struct vfs_backend base;
    void *address;
    unsigned int limit;

};

void kernel_setup_modules(unsigned int count, struct kernel_module *modules);
struct container *kernel_setup();
