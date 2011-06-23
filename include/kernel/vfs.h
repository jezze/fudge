#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

struct file_node
{

    unsigned int id;
    char name[128];
    unsigned int length;
    void (*open)(struct file_node *node);
    void (*close)(struct file_node *node);
    unsigned int (*read)(struct file_node *node, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct file_node *node, unsigned int offset, unsigned int count, void *buffer);
    struct file_node *(*walk)(struct file_node *node, unsigned int index);

};

struct file_descriptor
{

    struct file_node *node;
    unsigned int permissions;

};

extern unsigned int vfs_open(char *name);
extern void vfs_close(unsigned int index);
extern struct file_node *vfs_get(unsigned int index);
extern struct file_node *vfs_find(struct file_node *node, char *name);
extern struct file_node *vfs_get_root();
extern struct file_node *vfs_add_node(char *name, unsigned int length);
extern void vfs_init();

#endif

