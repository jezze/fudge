struct binary_interface
{

    struct binary_interface *sibling;
    unsigned int (*match)(struct vfs_interface *interface, unsigned int id);
    unsigned int (*find_symbol)(struct vfs_interface *interface, unsigned int id, unsigned int count, const char *symbol);
    unsigned int (*copy_program)(struct vfs_interface *interface, unsigned int id);
    unsigned int (*relocate)(struct vfs_interface *interface, unsigned int id, unsigned int address);

};

struct binary_interface *binary_get_interface(struct vfs_interface *interface, unsigned int id);
void binary_register_interface(struct binary_interface *interface);
void binary_unregister_interface(struct binary_interface *interface);
void binary_init_interface(struct binary_interface *interface, unsigned int (*match)(struct vfs_interface *interface, unsigned int id), unsigned int (*find_symbol)(struct vfs_interface *interface, unsigned int id, unsigned int count, const char *symbol), unsigned int (*copy_program)(struct vfs_interface *interface, unsigned int id), unsigned int (*relocate)(struct vfs_interface *interface, unsigned int id, unsigned int address));
void binary_setup();
