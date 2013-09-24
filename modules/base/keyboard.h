struct base_keyboard_interface
{

    unsigned char keymap[256];
    void (*open_data)(struct base_device *device, enum vfs_state *state);
    void (*close_data)(struct base_device *device, enum vfs_state *state);
    unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);

};

void base_keyboard_register_interface(struct base_keyboard_interface *interface, struct base_device *device);
void base_keyboard_init_interface(struct base_keyboard_interface *interface, void (*open_data)(struct base_device *device, enum vfs_state *state), void (*close_data)(struct base_device *device, enum vfs_state *state), unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer));
void base_keyboard_setup();
