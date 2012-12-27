struct nodefs_filesystem
{

    struct vfs_interface base;
    struct nodefs_node *nodes;

};

void nodefs_init_filesystem(struct nodefs_filesystem *filesystem);
