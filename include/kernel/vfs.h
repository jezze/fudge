#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

struct vfs_node;

extern struct vfs_node *vfs_init(unsigned int location);

#endif

