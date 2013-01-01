#define SYSTEM_FILESYSTEM_SLOTS         32

struct system_filesystem
{

    struct vfs_interface base;
    struct system_group root;
    unsigned int (*readers[SYSTEM_FILESYSTEM_SLOTS])(struct system_node *node, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*writers[SYSTEM_FILESYSTEM_SLOTS])(struct system_node *node, unsigned int offset, unsigned int count, void *buffer);

};

void system_init_filesystem(struct system_filesystem *filesystem);
