struct module_backend
{

    struct vfs_backend base;
    void *address;
    unsigned int size;

};

struct vfs_backend *vfs_module_setup(unsigned int modulesc, void **modulesv);
