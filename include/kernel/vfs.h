#ifndef VFS_H
#define VFS_H

#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02

struct vfs_node;

typedef void (*vfs_open_t)(struct vfs_node*);
typedef void (*vfs_close_t)(struct vfs_node*);
typedef uint32_t (*vfs_read_t)(struct vfs_node*, uint32_t offset, uint32_t size, char *buffer);
typedef uint32_t (*vfs_write_t)(struct vfs_node*, uint32_t offset, uint32_t size, char *buffer);
typedef struct vfs_node * (*vfs_walk_t)(struct vfs_node*, uint32_t);
typedef struct vfs_node * (*vfs_find_t)(struct vfs_node*, char *name);

typedef struct vfs_node
{

    char name[128];
    uint32_t inode;
    uint32_t flags;
    uint32_t length;
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
extern uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
extern uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
extern vfs_node_t *vfs_walk(vfs_node_t *node, uint32_t index);
extern vfs_node_t *vfs_find(vfs_node_t *node, char *name);

#endif

