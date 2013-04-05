#define TEMP_BUFFER_SLOTS               4

struct temp_filesystem
{

    struct vfs_interface base;
    struct circular_stream buffers[TEMP_BUFFER_SLOTS];

};

void temp_init_filesystem(struct temp_filesystem *filesystem);
