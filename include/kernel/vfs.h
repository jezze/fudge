#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#define VFS_FILESYSTEM_MAX 8

struct vfs_node;
struct vfs_node_operations;

struct vfs_node_operations
{

    void (*open)(struct vfs_node *self);
    void (*close)(struct vfs_node *self);
    unsigned int (*read)(struct vfs_node *self, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_node *self, unsigned int count, void *buffer);

};

struct vfs_node
{

    unsigned int id;
    struct vfs_node_operations operations;
    void *physical;

};

struct vfs_descriptor
{

    unsigned int index;
    struct vfs_node *node;
    unsigned int permissions;

};

struct vfs_filesystem
{

    struct vfs_node *root;
    struct vfs_node *(*lookup)(struct vfs_filesystem *self, char *path);

};

extern void vfs_register_filesystem(struct vfs_filesystem *filesystem);
extern void vfs_register_node(struct vfs_node *node);
extern struct vfs_node *vfs_find(char *path);
extern void vfs_init();

#endif

