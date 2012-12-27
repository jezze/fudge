struct system_filesystem
{

    struct vfs_interface base;
    struct system_group *groups;

};

void system_init_filesystem(struct system_filesystem *filesystem);
