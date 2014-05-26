#define VFS_RESOURCE_BACKEND            1001
#define VFS_RESOURCE_PROTOCOL           1002
#define VFS_RESOURCE_CHANNEL            1003
#define VFS_RESOURCE_DESCRIPTOR         1004
#define VFS_RESOURCE_MOUNT              1005

struct vfs_backend
{

    struct resource_item resource;
    unsigned int id;
    unsigned int (*read)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer);

};

struct vfs_protocol
{

    struct resource_item resource;
    unsigned int (*match)(struct vfs_backend *backend);
    unsigned int (*root)(struct vfs_backend *backend);
    unsigned int (*open)(struct vfs_backend *backend, unsigned int id);
    unsigned int (*close)(struct vfs_backend *backend, unsigned int id);
    unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*parent)(struct vfs_backend *backend, unsigned int id);
    unsigned int (*child)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path);
    unsigned long (*get_physical)(struct vfs_backend *backend, unsigned int id);

};

struct vfs_channel
{

    struct resource_item resource;
    struct vfs_backend *backend;
    struct vfs_protocol *protocol;

};

struct vfs_mount
{

    struct resource_item resource;
    struct {struct vfs_channel *channel; unsigned int id;} parent;
    struct {struct vfs_channel *channel; unsigned int id;} child;

};

struct vfs_descriptor
{

    struct resource_item resource;
    struct vfs_channel *channel;
    unsigned int id;
    unsigned int active;

};

struct vfs_backend *vfs_find_backend(unsigned int id);
struct vfs_protocol *vfs_find_protocol(struct vfs_backend *backend);
void vfs_init_backend(struct vfs_backend *backend, unsigned int id, unsigned int (*read)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer));
void vfs_init_protocol(struct vfs_protocol *protocol, unsigned int (*match)(struct vfs_backend *backend), unsigned int (*root)(struct vfs_backend *backend), unsigned int (*open)(struct vfs_backend *backend, unsigned int id), unsigned int (*close)(struct vfs_backend *backend, unsigned int id), unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*parent)(struct vfs_backend *backend, unsigned int id), unsigned int (*child)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path), unsigned long (*get_physical)(struct vfs_backend *backend, unsigned int id));
void vfs_init_channel(struct vfs_channel *channel, struct vfs_backend *backend, struct vfs_protocol *protocol);
void vfs_init_mount(struct vfs_mount *mount, struct vfs_channel *pchannel, unsigned int pid, struct vfs_channel *cchannel, unsigned int cid);
void vfs_init_descriptor(struct vfs_descriptor *descriptor, struct vfs_channel *channel, unsigned int id);
void vfs_setup_cpio(struct vfs_protocol *protocol);
void vfs_setup_tar(struct vfs_protocol *protocol);
