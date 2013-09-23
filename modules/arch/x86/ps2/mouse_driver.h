struct ps2_mouse_driver
{

    struct base_driver base;
    struct base_mouse_interface imouse;
    unsigned char cycle;
    char status;

};

void ps2_init_mouse_driver(struct ps2_mouse_driver *driver);
