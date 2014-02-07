struct ps2_keyboard_stream
{

    char buffer[512];
    unsigned int head;
    unsigned int tail;

};

struct ps2_keyboard_driver
{

    struct base_driver base;
    struct base_keyboard_interface ikeyboard;
    struct ps2_keyboard_stream stream;
    struct rendezvous rdata;
    unsigned int escaped;
    unsigned int ctrl;
    unsigned int alt;
    unsigned int shift;

};

void ps2_init_keyboard_driver(struct ps2_keyboard_driver *driver);
