#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#define VFS_FILESYSTEM_SLOTS 8

struct vfs_node
{

    char name[256];
    void *physical;
    unsigned int (*read)(struct vfs_node *self, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_node *self, unsigned int count, void *buffer);

};

struct vfs_filesystem
{

    char *name;
    unsigned int firstIndex;
    void (*open)(struct vfs_filesystem *self, unsigned int index);
    void (*close)(struct vfs_filesystem *self, unsigned int index);
    unsigned int (*read)(struct vfs_filesystem *self, unsigned int index, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_filesystem *self, unsigned int index, unsigned int count, void *buffer);
    char *(*get_name)(struct vfs_filesystem *self, unsigned int index);
    unsigned int (*find_node)(struct vfs_filesystem *self, char *name);
    unsigned int (*walk)(struct vfs_filesystem *self, unsigned int index);
    void *(*get_physical)(struct vfs_filesystem *self, unsigned int index);

};

void vfs_register_filesystem(struct vfs_filesystem *filesystem);
struct vfs_filesystem *vfs_get_filesystem(unsigned int index);
struct vfs_filesystem *vfs_find_filesystem(char *path);
void vfs_node_init(struct vfs_node *node, char *name, unsigned int (*read)(struct vfs_node *self, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_node *self, unsigned int count, void *buffer));
void vfs_filesystem_init(struct vfs_filesystem *filesystem, char *name, unsigned int firstIndex, void (*open)(struct vfs_filesystem *self, unsigned int index), void (*close)(struct vfs_filesystem *self, unsigned int index), unsigned int (*read)(struct vfs_filesystem *self, unsigned int index, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_filesystem *self, unsigned int index, unsigned int count, void *buffer), char *(*get_name)(struct vfs_filesystem *self, unsigned int index), unsigned int (*find_node)(struct vfs_filesystem *self, char *name), unsigned int (*walk)(struct vfs_filesystem *self, unsigned int index), void *(*get_physical)(struct vfs_filesystem *self, unsigned int index));

#endif

