struct base_terminal_interface
{

    struct base_interface base;
    unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);

};

void base_terminal_register_interface(struct base_terminal_interface *interface, struct base_device *device);
void base_terminal_init_interface(struct base_terminal_interface *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer));
void base_terminal_setup();
