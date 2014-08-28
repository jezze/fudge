struct base_terminal_interface
{

    struct base_interface base;
    unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);

};

struct base_terminal_node
{

    struct system_node base;
    struct system_node data;
    struct base_device *device;
    struct base_terminal_interface *interface;

};

void base_terminal_register_interface(struct base_terminal_interface *interface);
void base_terminal_register_node(struct base_terminal_node *node);
void base_terminal_unregister_interface(struct base_terminal_interface *interface);
void base_terminal_unregister_node(struct base_terminal_node *node);
void base_terminal_init_interface(struct base_terminal_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer));
void base_terminal_init_node(struct base_terminal_node *node, struct base_device *device, struct base_terminal_interface *interface);
void base_terminal_setup();
