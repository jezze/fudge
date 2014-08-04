struct base_network_interface
{

    struct base_interface base;
    unsigned int (*receive)(struct base_bus *bus, unsigned int id, unsigned int count, void *buffer);
    unsigned int (*send)(struct base_bus *bus, unsigned int id, unsigned int count, void *buffer);
    void *(*get_packet)(struct base_bus *bus, unsigned int id);
    void (*dump_packet)(struct base_bus *bus, unsigned int id);
    unsigned char mac[6];

};

struct base_network_protocol
{

    char *name;
    unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer);

};

struct base_network_node
{

    struct system_group base;
    struct system_stream data;
    struct system_stream mac;
    struct base_device *device;
    struct base_network_interface *interface;

};

void base_network_register_interface(struct base_network_interface *interface);
void base_network_register_protocol(struct base_network_protocol *protocol);
void base_network_register_node(struct base_network_node *node);
void base_network_unregister_interface(struct base_network_interface *interface);
void base_network_unregister_protocol(struct base_network_protocol *protocol);
void base_network_unregister_node(struct base_network_node *node);
void base_network_init_interface(struct base_network_interface *interface, unsigned int (*receive)(struct base_bus *bus, unsigned int id, unsigned int count, void *buffer), unsigned int (*send)(struct base_bus *bus, unsigned int id, unsigned int count, void *buffer), void *(*get_packet)(struct base_bus *bus, unsigned int id), void (*dump_packet)(struct base_bus *bus, unsigned int id));
void base_network_init_protocol(struct base_network_protocol *protocol, char *name, unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer));
void base_network_init_node(struct base_network_node *node, struct base_device *device, struct base_network_interface *interface);
void base_network_setup();
