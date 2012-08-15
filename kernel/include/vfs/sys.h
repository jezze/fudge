#ifndef KERNEL_VFS_SYS_H
#define KERNEL_VFS_SYS_H

struct vfs_sys_filesystem
{

    struct modules_filesystem base;
    struct modules_base **modules;

};

void vfs_sys_filesystem_init(struct vfs_sys_filesystem *filesystem, struct modules_base **modules);

#endif

