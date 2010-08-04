#ifndef LIB_VFS_H
#define LIB_VFS_H

struct vfs_node;

typedef void (*vfs_open_t)(struct vfs_node*);
typedef void (*vfs_close_t)(struct vfs_node*);
typedef unsigned int (*vfs_read_t)(struct vfs_node*, unsigned int offset, unsigned int count, void *buffer);
typedef unsigned int (*vfs_write_t)(struct vfs_node*, unsigned int offset, unsigned int count, void *buffer);
typedef struct vfs_node * (*vfs_walk_t)(struct vfs_node*, unsigned int index);

struct vfs_node
{

    char name[128];
    unsigned int inode;
    unsigned int length;
    vfs_open_t open;
    vfs_close_t close;
    vfs_read_t read;
    vfs_write_t write;
    vfs_walk_t walk;

};

extern void vfs_open(struct vfs_node *node);
extern void vfs_close(struct vfs_node *node);
extern unsigned int vfs_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer);
extern unsigned int vfs_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer);
extern struct vfs_node *vfs_walk(struct vfs_node *node, unsigned int index);
extern struct vfs_node *vfs_find(struct vfs_node *node, char *name);

#endif

