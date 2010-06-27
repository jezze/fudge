#ifndef VFS_H
#define VFS_H

#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02

struct vfs_node;

typedef void (*vfs_open_t)(struct vfs_node*);
typedef void (*vfs_close_t)(struct vfs_node*);
typedef unsigned int (*vfs_read_t)(struct vfs_node*, unsigned int offset, unsigned int size, char *buffer);
typedef unsigned int (*vfs_write_t)(struct vfs_node*, unsigned int offset, unsigned int size, char *buffer);
typedef struct vfs_node * (*vfs_walk_t)(struct vfs_node*, unsigned int index);
typedef struct vfs_node * (*vfs_find_t)(struct vfs_node*, char *name);

typedef struct vfs_node
{

    char name[128];
    unsigned int inode;
    unsigned int flags;
    unsigned int length;
    vfs_open_t open;
    vfs_close_t close;
    vfs_read_t read;
    vfs_write_t write;
    vfs_walk_t walk;
    vfs_find_t find;

} vfs_node_t;

extern vfs_node_t *fsRoot;

extern void vfs_open(vfs_node_t *node);
extern void vfs_close(vfs_node_t *node);
extern unsigned int vfs_read(vfs_node_t *node, unsigned int offset, unsigned int size, char *buffer);
extern unsigned int vfs_write(vfs_node_t *node, unsigned int offset, unsigned int size, char *buffer);
extern vfs_node_t *vfs_walk(vfs_node_t *node, unsigned int index);
extern vfs_node_t *vfs_find(vfs_node_t *node, char *name);

#endif

