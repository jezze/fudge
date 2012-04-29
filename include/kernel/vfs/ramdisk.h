#ifndef KERNEL_VFS_RAMDISK_H
#define KERNEL_VFS_RAMDISK_H

struct ramdisk_node;

void vfs_ramdisk_init(struct ramdisk_node *n, unsigned int c);

#endif

