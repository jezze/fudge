struct system_backend
{

    struct vfs_backend base;
    struct system_node root;
    struct system_header header;

};

void system_initbackend(struct system_backend *backend);
