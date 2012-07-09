#ifndef KERNEL_VFS_RAMDISK_H
#define KERNEL_VFS_RAMDISK_H

struct vfs_ramdisk_filesystem
{

    struct modules_filesystem base;
    struct ramdisk_image *image;

};

void vfs_ramdisk_filesystem_init(struct vfs_ramdisk_filesystem *filesystem, struct ramdisk_image *image);
void vfs_ramdisk_setup(struct ramdisk_image *image);

#endif

