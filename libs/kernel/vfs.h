struct vfs_interface
{

    unsigned int rootid;
    void (*open)(struct vfs_interface *self, unsigned int id);
    void (*close)(struct vfs_interface *self, unsigned int id);
    unsigned int (*read)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*walk)(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path);
    unsigned int (*get_physical)(struct vfs_interface *self, unsigned int id);

};

void vfs_init_interface(struct vfs_interface *interface, unsigned int rootid, char *name, void (*open)(struct vfs_interface *self, unsigned int id), void (*close)(struct vfs_interface *self, unsigned int id), unsigned int (*read)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*walk)(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path), unsigned int (*get_physical)(struct vfs_interface *self, unsigned int id));
struct vfs_interface *vfs_setup();
