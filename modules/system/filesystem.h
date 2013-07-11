#define SYSTEM_ROUTINES                 32

struct system_backend
{

    struct vfs_backend base;
    struct system_group root;
    unsigned int (*readers[SYSTEM_ROUTINES])(struct system_node *node, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*writers[SYSTEM_ROUTINES])(struct system_node *node, unsigned int offset, unsigned int count, void *buffer);

};

void system_init_backend(struct system_backend *backend);
void system_init_protocol(struct vfs_protocol *protocol, struct system_backend *backend);
