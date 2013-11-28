struct base_mouse_interface
{

    char vx;
    char vy;
    unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);

};

void base_mouse_register_interface(struct base_mouse_interface *interface, struct base_device *device);
void base_mouse_init_interface(struct base_mouse_interface *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer));
void base_mouse_setup();
