#ifndef FS_H
#define FS_H

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08

struct fs_node;

typedef uint32_t (*read_type_t)(struct fs_node*, uint32_t offset, uint32_t size, char *buffer);
typedef uint32_t (*write_type_t)(struct fs_node*, uint32_t offset, uint32_t size, char *buffer);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct directory_entry * (*readdir_type_t)(struct fs_node*, uint32_t);
typedef struct fs_node * (*finddir_type_t)(struct fs_node*, char *name);

typedef struct fs_node
{

    char name[128];
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode;
    uint32_t length;
    uint32_t impl;
    read_type_t read;
    read_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    struct fs_node *ptr;

} fs_node_t;

typedef struct directory_entry
{

    char name[128];
    uint32_t ino;

} directory_entry_t;

extern fs_node_t *fsRoot;

extern uint32_t fs_read(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
extern uint32_t fs_write(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
extern void fs_open(fs_node_t *node, uint8_t read, uint8_t write);
extern void fs_close(fs_node_t *node);
extern directory_entry_t *fs_directory_read(fs_node_t *node, uint32_t index);
extern fs_node_t *fs_directory_find(fs_node_t *node, char *name);

#endif

