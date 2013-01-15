#define RAMDISK_HEADER_SLOTS            128

struct ramdisk_filesystem
{

    struct vfs_interface interface;
    struct {struct tar_header *headers[RAMDISK_HEADER_SLOTS]; unsigned int count;} image;

};

void ramdisk_init_filesystem(struct ramdisk_filesystem *filesystem);
struct vfs_interface *ramdisk_setup(int ramdiskc, void **ramdiskv);
