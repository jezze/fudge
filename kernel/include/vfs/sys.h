#ifndef KERNEL_VFS_SYS_H
#define KERNEL_VFS_SYS_H

struct vfs_sys_filesystem
{

    struct modules_filesystem base;
    union modules_module **modules;

};

void vfs_sys_filesystem_init(struct vfs_sys_filesystem *filesystem, union modules_module **modules);
struct modules_filesystem *vfs_sys_setup(union modules_module **modules);

#endif

