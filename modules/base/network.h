struct base_network_interface
{

    struct base_interface base;
    unsigned int (*receive)(struct base_device *device, unsigned int count, void *buffer);
    unsigned int (*send)(struct base_device *device, unsigned int count, void *buffer);
    void *(*get_packet)(struct base_device *device);
    void (*dump_packet)(struct base_device *device);
    unsigned char mac[6];

};

struct base_network_protocol
{

    char *name;
    unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer);

};

void base_network_register_interface(struct base_network_interface *interface, struct base_device *device);
void base_network_register_protocol(struct base_network_protocol *protocol);
void base_network_init_interface(struct base_network_interface *interface, unsigned int (*receive)(struct base_device *device, unsigned int count, void *buffer), unsigned int (*send)(struct base_device *device, unsigned int count, void *buffer), void *(*get_packet)(struct base_device *device), void (*dump_packet)(struct base_device *device));
void base_network_init_protocol(struct base_network_protocol *protocol, char *name, unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer));
void base_network_setup();
