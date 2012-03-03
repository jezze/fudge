#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#define VFS_FILESYSTEM_SLOTS 8

struct vfs_filesystem
{

    char *name;
    void (*open)(struct vfs_filesystem *self, unsigned int id);
    void (*close)(struct vfs_filesystem *self, unsigned int id);
    unsigned int (*read)(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer);
    unsigned int (*find)(struct vfs_filesystem *self, char *name);
    unsigned int (*walk)(struct vfs_filesystem *self, unsigned int id);
    char *(*get_name)(struct vfs_filesystem *self, unsigned int id);
    void *(*get_physical)(struct vfs_filesystem *self, unsigned int id);

};

void vfs_register_filesystem(struct vfs_filesystem *filesystem);
struct vfs_filesystem *vfs_get_filesystem(unsigned int index);
struct vfs_filesystem *vfs_find_filesystem(char *path);
void vfs_filesystem_init(struct vfs_filesystem *filesystem, char *name, void (*open)(struct vfs_filesystem *self, unsigned int id), void (*close)(struct vfs_filesystem *self, unsigned int id), unsigned int (*read)(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer), unsigned int (*find_node)(struct vfs_filesystem *self, char *name), unsigned int (*walk)(struct vfs_filesystem *self, unsigned int id), char *(*get_name)(struct vfs_filesystem *self, unsigned int id), void *(*get_physical)(struct vfs_filesystem *self, unsigned int id));

#endif

