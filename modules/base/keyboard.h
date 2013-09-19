struct base_keyboard
{

    unsigned char keymap[256];
    unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);

};

void base_register_keyboard(struct base_keyboard *interface, struct base_device *device);
void base_init_keyboard(struct base_keyboard *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer));
void base_setup_keyboard();
