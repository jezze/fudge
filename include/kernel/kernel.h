#ifndef KERNEL_H
#define KERNEL_H

struct vfs_node_t;

typedef struct kernel
{

} kernel_t;

extern kernel_t kernel;
extern vfs_node_t *fsRoot;

extern unsigned int kernel_reboot();

#endif

