struct binary_format
{

    struct binary_format *sibling;
    unsigned int (*match)(struct vfs_interface *interface, unsigned int id);
    unsigned int (*find_symbol)(struct vfs_interface *interface, unsigned int id, unsigned int count, const char *symbol);
    unsigned int (*copy_program)(struct vfs_interface *interface, unsigned int id);
    unsigned int (*relocate)(struct vfs_interface *interface, unsigned int id, unsigned int address);

};

struct binary_format *binary_get_format(struct vfs_interface *interface, unsigned int id);
void binary_register_format(struct binary_format *format);
void binary_unregister_format(struct binary_format *format);
void binary_init_format(struct binary_format *format, unsigned int (*match)(struct vfs_interface *interface, unsigned int id), unsigned int (*find_symbol)(struct vfs_interface *interface, unsigned int id, unsigned int count, const char *symbol), unsigned int (*copy_program)(struct vfs_interface *interface, unsigned int id), unsigned int (*relocate)(struct vfs_interface *interface, unsigned int id, unsigned int address));
void binary_setup();
