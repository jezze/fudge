#ifndef MODULES_NODEFS_H
#define MODULES_NODEFS_H

#define NODEFS_DRIVER_TYPE 0x6792

struct nodefs_node
{

    char *name;
    unsigned int (*read)(struct nodefs_node *self, unsigned int count, void *buffer);
    unsigned int (*write)(struct nodefs_node *self, unsigned int count, void *buffer);

};

struct nodefs_filesystem
{

    struct vfs_filesystem base;
    struct nodefs_node *nodes[128];
    unsigned int count;

};

struct nodefs_driver
{

    struct modules_driver base;
    struct nodefs_filesystem filesystem;
    void (*register_node)(struct nodefs_driver *self, struct nodefs_node *node);
    void (*unregister_node)(struct nodefs_driver *self, struct nodefs_node *node);

};

void nodefs_driver_init(struct nodefs_driver *driver);

#endif

