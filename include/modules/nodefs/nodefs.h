#ifndef MODULES_NODEFS_H
#define MODULES_NODEFS_H

#define NODEFS_DRIVER_TYPE 0x0423

struct nodefs_driver
{

    struct modules_driver base;
    struct vfs_filesystem filesystem;
    struct vfs_node *nodes[128];
    void (*register_node)(struct nodefs_driver *self, struct vfs_node *node);
    void (*unregister_node)(struct nodefs_driver *self, struct vfs_node *node);

};

extern void nodefs_driver_init(struct nodefs_driver *driver);

#endif

