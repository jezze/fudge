#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

struct file_node;

extern unsigned int vfs_open(char *name, unsigned int permissions);
extern void vfs_close(unsigned int index);
extern struct file_node *vfs_find(struct file_node *node, char *name);
extern struct file_node *vfs_get_root();
extern struct file_node *vfs_add_node(char *name, unsigned int length);
extern void vfs_init();

#endif

