struct net_interface
{

    struct base_driver *driver;
    char mac[6];
    unsigned int (*send)(struct net_interface *self, unsigned int count, void *buffer);

};

struct net_protocol
{

    char *name;
    void (*handle_read)(struct net_protocol *self, unsigned int count, void *buffer);
    unsigned int (*read)(struct net_protocol *self, struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct net_protocol *self, struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer);

};

void net_register_interface(struct net_interface *interface);
void net_register_protocol(unsigned short index, struct net_protocol *protocol);
void net_unregister_interface(struct net_interface *interface);
void net_unregister_protocol(unsigned short index);
void net_init_interface(struct net_interface *interface, struct base_driver *driver, unsigned int (*send)(struct net_interface *self, unsigned int count, void *buffer));
