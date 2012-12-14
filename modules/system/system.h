struct system_filesystem
{

    struct vfs_interface base;

};

void system_init_filesystem(struct system_filesystem *filesystem);
