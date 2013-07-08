struct vfs_backend
{

    unsigned int (*read)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer);

};

struct vfs_protocol
{

    struct vfs_protocol *sibling;
    unsigned int rootid;
    unsigned int (*match)(struct vfs_backend *backend);
    unsigned int (*open)(struct vfs_backend *backend, unsigned int id);
    unsigned int (*close)(struct vfs_backend *backend, unsigned int id);
    unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*parent)(struct vfs_backend *backend, unsigned int id);
    unsigned int (*walk)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path);
    unsigned int (*get_physical)(struct vfs_backend *backend, unsigned int id);

};

struct vfs_session
{

    struct vfs_backend *backend;
    struct vfs_protocol *protocol;

};

unsigned int vfs_findnext(unsigned int count, const char *path);
unsigned int vfs_isparent(unsigned int count, const char *path);
struct vfs_protocol *vfs_get_protocol(struct vfs_backend *backend);
void vfs_register_protocol(struct vfs_protocol *protocol);
void vfs_unregister_protocol(struct vfs_protocol *protocol);
void vfs_init_backend(struct vfs_backend *backend, unsigned int (*read)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer));
void vfs_init_protocol(struct vfs_protocol *protocol, unsigned int rootid, unsigned int (*match)(struct vfs_backend *backend), unsigned int (*open)(struct vfs_backend *backend, unsigned int id), unsigned int (*close)(struct vfs_backend *backend, unsigned int id), unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*parent)(struct vfs_backend *backend, unsigned int id), unsigned int (*walk)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path), unsigned int (*get_physical)(struct vfs_backend *backend, unsigned int id));
void vfs_setup();
struct vfs_protocol *vfs_cpio_setup();
struct vfs_protocol *vfs_tar_setup();
