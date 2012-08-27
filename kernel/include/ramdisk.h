#ifndef KERNEL_RAMDISK_H
#define KERNEL_RAMDISK_H

#define RAMDISK_HEADER_SLOTS 128

struct ramdisk_image
{

    struct tar_header *headers[RAMDISK_HEADER_SLOTS];
    unsigned int count;
    unsigned int (*parse)(struct ramdisk_image *self, void *address);

};

struct ramdisk_filesystem
{

    struct vfs_interface interface;
    struct ramdisk_image *image;

};

void ramdisk_image_init(struct ramdisk_image *image);
void ramdisk_filesystem_init(struct ramdisk_filesystem *filesystem, struct ramdisk_image *image);

#endif

