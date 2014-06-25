struct base_terminal_interface
{

    struct base_interface base;
    unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    struct rendezvous rdata;
    struct rendezvous wdata;

};

void base_terminal_register_interface(struct base_terminal_interface *interface, struct base_bus *bus, unsigned int id);
void base_terminal_unregister_interface(struct base_terminal_interface *interface);
void base_terminal_init_interface(struct base_terminal_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer));
void base_terminal_setup();
