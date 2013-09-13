struct base_mouse
{

    struct system_group base;
    char vx;
    char vy;

};

void base_register_mouse(struct base_mouse *interface, struct base_device *device);
void base_init_mouse(struct base_mouse *interface);
void base_setup_mouse();
