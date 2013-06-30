struct kernel_module
{

    void *base;
    unsigned int size;

};

struct runtime_container *kernel_setup(unsigned int count, struct kernel_module *modules);
