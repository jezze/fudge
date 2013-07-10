struct kernel_module
{

    struct vfs_backend base;
    void *address;
    unsigned int limit;

};

struct container *kernel_setup(unsigned int count, struct kernel_module *modules);
