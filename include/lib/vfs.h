#ifndef VFS_H
#define VFS_H

struct vfs_node;

typedef void (*vfs_open_t)(struct vfs_node*);
typedef void (*vfs_close_t)(struct vfs_node*);
typedef unsigned int (*vfs_read_t)(struct vfs_node*, unsigned int offset, unsigned int count, void *buffer);
typedef unsigned int (*vfs_write_t)(struct vfs_node*, unsigned int offset, unsigned int count, void *buffer);
typedef struct vfs_node * (*vfs_walk_t)(struct vfs_node*, unsigned int index);

typedef struct vfs_node
{

    char name[128];
    unsigned int inode;
    unsigned int length;
    vfs_open_t open;
    vfs_close_t close;
    vfs_read_t read;
    vfs_write_t write;
    vfs_walk_t walk;

} vfs_node_t;

extern void vfs_open(vfs_node_t *node);
extern void vfs_close(vfs_node_t *node);
extern unsigned int vfs_read(vfs_node_t *node, unsigned int offset, unsigned int count, void *buffer);
extern unsigned int vfs_write(vfs_node_t *node, unsigned int offset, unsigned int count, void *buffer);
extern vfs_node_t *vfs_walk(vfs_node_t *node, unsigned int index);
extern vfs_node_t *vfs_find(vfs_node_t *node, char *name);

#endif

