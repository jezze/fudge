struct base_mouse_interface
{

    char vx;
    char vy;

};

void base_mouse_register_interface(struct base_mouse_interface *interface, struct base_device *device);
void base_mouse_init_interface(struct base_mouse_interface *interface);
void base_mouse_setup();
