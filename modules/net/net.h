struct net_protocol
{

    char *name;
    unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer);

};

void net_register_protocol(unsigned short index, struct net_protocol *protocol);
void net_unregister_protocol(unsigned short index);
void net_init_protocol(struct net_protocol *protocol, char *name, unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer));
