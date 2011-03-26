#ifndef LIB_VFS_H
#define LIB_VFS_H

struct vfs_node
{

    char name[128];
    unsigned int inode;
    unsigned int length;
    void (*open)(struct vfs_node *node);
    void (*close)(struct vfs_node *node);
    unsigned int (*read)(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer);
    struct vfs_node *(*walk)(struct vfs_node *node, unsigned int index);

};

#endif

