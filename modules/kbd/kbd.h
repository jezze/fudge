struct kbd_interface
{

    struct system_group base;
    unsigned char keymap[256];
    unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);

};

void kbd_register_device(struct kbd_interface *interface, struct base_device *device);
void kbd_init_interface(struct kbd_interface *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer));
