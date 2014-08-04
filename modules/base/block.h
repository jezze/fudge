struct base_block_interface
{

    struct base_interface base;
    unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);

};

struct base_block_protocol
{

    char *name;

};

struct base_block_node
{

    struct system_group base;
    struct system_stream data;
    struct base_device *device;
    struct base_block_interface *interface;

};

void base_block_register_interface(struct base_block_interface *interface);
void base_block_register_protocol(struct base_block_protocol *protocol);
void base_block_register_node(struct base_block_node *node);
void base_block_unregister_interface(struct base_block_interface *interface);
void base_block_unregister_protocol(struct base_block_protocol *protocol);
void base_block_unregister_node(struct base_block_node *node);
void base_block_init_interface(struct base_block_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer));
void base_block_init_protocol(struct base_block_protocol *protocol, char *name);
void base_block_init_node(struct base_block_node *node, struct base_device *device, struct base_block_interface *interface);
void base_block_setup();
