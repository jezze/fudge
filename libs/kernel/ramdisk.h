#define RAMDISK_HEADER_SLOTS            128

struct ramdisk_filesystem
{

    struct vfs_interface interface;
    struct {struct tar_header *headers[RAMDISK_HEADER_SLOTS]; unsigned int count;} image;

};

unsigned int ramdisk_parse(struct ramdisk_filesystem *filesystem, void *address);
void ramdisk_init_filesystem(struct ramdisk_filesystem *filesystem);
