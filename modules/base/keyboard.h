struct base_keyboard_interface
{

    struct base_interface base;
    unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*read_keymap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_keymap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);

};

struct base_keyboard_node
{

    struct system_node base;
    struct system_node data;
    struct system_node keymap;
    struct base_device *device;
    struct base_keyboard_interface *interface;

};

void base_keyboard_register_interface(struct base_keyboard_interface *interface);
void base_keyboard_register_node(struct base_keyboard_node *node);
void base_keyboard_unregister_interface(struct base_keyboard_interface *interface);
void base_keyboard_unregister_node(struct base_keyboard_node *node);
void base_keyboard_init_interface(struct base_keyboard_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_keymap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_keymap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer));
void base_keyboard_init_node(struct base_keyboard_node *node, struct base_device *device, struct base_keyboard_interface *interface);
void base_keyboard_setup();
