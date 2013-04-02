#define TEMP_FILESYSTEM_SLOTS           4

struct temp_filesystem
{

    struct vfs_interface base;
    struct temp_buffer buffers[TEMP_FILESYSTEM_SLOTS];

};

void temp_init_filesystem(struct temp_filesystem *filesystem);
