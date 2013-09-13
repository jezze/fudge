struct system_backend
{

    struct vfs_backend base;
    struct system_group root;

};

void system_init_backend(struct system_backend *backend);
