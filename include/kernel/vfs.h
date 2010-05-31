#ifndef VFS_H
#define VFS_H

#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02

struct vfs_node;

typedef uint32_t (*read_type_t)(struct vfs_node*, uint32_t offset, uint32_t size, char *buffer);
typedef uint32_t (*write_type_t)(struct vfs_node*, uint32_t offset, uint32_t size, char *buffer);
typedef void (*open_type_t)(struct vfs_node*);
typedef void (*close_type_t)(struct vfs_node*);
typedef struct vfs_directory_entry * (*readdir_type_t)(struct vfs_node*, uint32_t);
typedef struct vfs_node * (*finddir_type_t)(struct vfs_node*, char *name);

typedef struct vfs_node
{

    char name[128];
    uint32_t flags;
    uint32_t inode;
    uint32_t length;
    read_type_t read;
    read_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;

} vfs_node_t;

typedef struct vfs_directory_entry
{

    char name[128];
    uint32_t ino;

} vfs_directory_entry_t;

extern vfs_node_t *fsRoot;

extern uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
extern uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
extern void vfs_open(vfs_node_t *node, uint8_t read, uint8_t write);
extern void vfs_close(vfs_node_t *node);
extern vfs_directory_entry_t *vfs_directory_read(vfs_node_t *node, uint32_t index);
extern vfs_node_t *vfs_directory_find(vfs_node_t *node, char *name);

#endif

