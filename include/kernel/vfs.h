#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

struct vfs_node;
struct vfs_node_operations;

struct vfs_node_operations
{

    void (*open)(struct vfs_node *node);
    void (*close)(struct vfs_node *node);
    unsigned int (*read)(struct vfs_node *node, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_node *node, unsigned int count, void *buffer);

};

struct vfs_node
{

    unsigned int id;
    struct vfs_node_operations operations;

};

struct vfs_descriptor
{

    struct vfs_node *node;
    unsigned int permissions;

};

struct vfs_filesystem
{

    char name[32];
    struct vfs_node *root;
    struct vfs_node *(*lookup)(struct vfs_filesystem *filesystem, char *path);

};

extern void vfs_register_filesystem(struct vfs_filesystem *filesystem);
extern struct vfs_node *vfs_find(char *path);
extern unsigned int vfs_open(char *path);
extern void vfs_close(unsigned int index);
extern struct vfs_descriptor *vfs_get_descriptor(unsigned int index);
extern void vfs_init();

#endif

