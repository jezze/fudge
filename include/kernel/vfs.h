#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#define VFS_FILESYSTEM_SLOTS 8
#define VFS_MOUNT_SLOTS 16

struct vfs_filesystem
{

    void (*open)(struct vfs_filesystem *self, unsigned int id);
    void (*close)(struct vfs_filesystem *self, unsigned int id);
    unsigned int (*read)(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*find)(struct vfs_filesystem *self, char *path);
    void *(*get_physical)(struct vfs_filesystem *self, unsigned int id);

};

struct vfs_mount
{

    struct vfs_filesystem *filesystem;
    char *path;

};

struct vfs_mount *vfs_find_mount(char *path);
void vfs_mount(struct vfs_filesystem *filesystem, char *path);
void vfs_filesystem_init(struct vfs_filesystem *filesystem, void (*open)(struct vfs_filesystem *self, unsigned int id), void (*close)(struct vfs_filesystem *self, unsigned int id), unsigned int (*read)(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*find_node)(struct vfs_filesystem *self, char *path), void *(*get_physical)(struct vfs_filesystem *self, unsigned int id));
void vfs_mount_init(struct vfs_mount *mount, struct vfs_filesystem *filesystem, char *path);
void vfs_init();

#endif

