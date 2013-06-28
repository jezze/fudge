struct binary_protocol
{

    struct binary_protocol *sibling;
    unsigned int (*match)(struct vfs_protocol *protocol, unsigned int id);
    unsigned int (*find_symbol)(struct vfs_protocol *protocol, unsigned int id, unsigned int count, const char *symbol);
    unsigned int (*copy_program)(struct vfs_protocol *protocol, unsigned int id);
    unsigned int (*relocate)(struct vfs_protocol *protocol, unsigned int id, unsigned int address);

};

struct binary_protocol *binary_get_protocol(struct vfs_protocol *protocol, unsigned int id);
void binary_register_protocol(struct binary_protocol *protocol);
void binary_unregister_protocol(struct binary_protocol *protocol);
void binary_init_protocol(struct binary_protocol *protocol, unsigned int (*match)(struct vfs_protocol *protocol, unsigned int id), unsigned int (*find_symbol)(struct vfs_protocol *protocol, unsigned int id, unsigned int count, const char *symbol), unsigned int (*copy_program)(struct vfs_protocol *protocol, unsigned int id), unsigned int (*relocate)(struct vfs_protocol *protocol, unsigned int id, unsigned int address));
void binary_setup();
