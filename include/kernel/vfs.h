#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

struct file_node;

extern struct file_node *vfs_get_root();
extern struct file_node *vfs_add_node(char *name, unsigned int length);
extern void vfs_init();

#endif

