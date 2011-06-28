#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

struct vfs_node
{

    unsigned int id;
    char name[128];
    unsigned int length;
    void (*open)(struct vfs_node *node);
    void (*close)(struct vfs_node *node);
    unsigned int (*read)(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer);
    struct vfs_node *(*walk)(struct vfs_node *node, unsigned int index);

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
extern unsigned int vfs_open(char *name);
extern void vfs_close(unsigned int index);
extern struct vfs_node *vfs_get(unsigned int index);
extern struct vfs_node *vfs_find(struct vfs_node *node, char *name);
extern struct vfs_node *vfs_find_root(char *name);
extern struct vfs_node *vfs_get_root();
extern struct vfs_node *vfs_add_node(char *name, unsigned int length);
extern void vfs_init();

#endif

