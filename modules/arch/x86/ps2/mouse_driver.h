struct ps2_mouse_stream
{

    char buffer[512];
    unsigned int head;
    unsigned int tail;

};

struct ps2_mouse_driver
{

    struct base_driver base;
    struct base_mouse_interface imouse;
    struct ps2_mouse_stream stream;
    unsigned char cycle;
    char status;

};

void ps2_init_mouse_driver(struct ps2_mouse_driver *driver);
