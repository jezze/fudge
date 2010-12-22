#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

struct vfs_node;

extern struct vfs_node *vfs_get_root();
extern struct vfs_node *vfs_add_node(char *name, unsigned int length);
extern void vfs_init();

#endif

