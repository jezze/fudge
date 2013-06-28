#define TEMP_BUFFER_SLOTS               8

struct temp_filesystem
{

    struct vfs_protocol base;

};

void temp_init_filesystem(struct temp_filesystem *filesystem);
