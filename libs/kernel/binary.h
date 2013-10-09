struct binary_protocol
{

    struct list_item item;
    unsigned int (*match)(struct vfs_session *session, unsigned int id);
    unsigned int (*find_symbol)(struct vfs_session *session, unsigned int id, unsigned int count, const char *symbol);
    unsigned int (*copy_program)(struct vfs_session *session, unsigned int id);
    unsigned int (*relocate)(struct vfs_session *session, unsigned int id, unsigned int address);

};

struct binary_protocol *binary_find_protocol(struct vfs_session *session, unsigned int id);
void binary_register_protocol(struct binary_protocol *protocol);
void binary_unregister_protocol(struct binary_protocol *protocol);
void binary_init_protocol(struct binary_protocol *protocol, unsigned int (*match)(struct vfs_session *session, unsigned int id), unsigned int (*find_symbol)(struct vfs_session *session, unsigned int id, unsigned int count, const char *symbol), unsigned int (*copy_program)(struct vfs_session *session, unsigned int id), unsigned int (*relocate)(struct vfs_session *session, unsigned int id, unsigned int address));
void binary_init_elf(struct binary_protocol *protocol);
void binary_setup();
