struct mouse_interface
{

    struct system_group base;
    char vx;
    char vy;

};

void mouse_register_device(struct mouse_interface *interface, struct base_device *device);
void mouse_init_interface(struct mouse_interface *interface);
