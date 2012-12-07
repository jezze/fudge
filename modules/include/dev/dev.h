struct dev_filesystem
{

    struct vfs_interface base;

};

void dev_init_filesystem(struct dev_filesystem *filesystem);
