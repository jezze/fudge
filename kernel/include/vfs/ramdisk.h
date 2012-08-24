#ifndef KERNEL_VFS_RAMDISK_H
#define KERNEL_VFS_RAMDISK_H

struct vfs_ramdisk_filesystem
{

    struct vfs_filesystem base;
    struct ramdisk_image *image;

};

void vfs_ramdisk_filesystem_init(struct vfs_ramdisk_filesystem *filesystem, struct ramdisk_image *image);

#endif

