struct base_keyboard_interface
{

    struct base_interface base;
    struct keycode {unsigned char length; unsigned char value[4];} keymap[256];
    unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int escaped;
    unsigned int ctrl;
    unsigned int alt;
    unsigned int shift;

};

void base_keyboard_connect_interface(struct base_keyboard_interface *interface, struct base_bus *bus, unsigned int id);
void base_keyboard_register_interface(struct base_keyboard_interface *interface);
void base_keyboard_unregister_interface(struct base_keyboard_interface *interface);
void base_keyboard_init_interface(struct base_keyboard_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer));
void base_keyboard_setup();
