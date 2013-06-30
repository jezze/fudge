struct module_backend
{

    struct vfs_backend base;
    struct kernel_module *module;

};

struct vfs_backend *vfs_module_setup(unsigned int count, struct kernel_module *modules);
