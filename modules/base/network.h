struct base_network_interface
{

    struct base_interface base;
    struct ctrl_networkctrl ctrl;
    unsigned int (*receive)(unsigned int count, void *buffer);
    unsigned int (*send)(unsigned int count, void *buffer);
    void *(*getpacket)();
    void (*dumppacket)();

};

struct base_network_interfacenode
{

    struct system_node base;
    struct system_node ctrl;
    struct system_node data;
    struct base_network_interface *interface;

};

struct base_network_protocol
{

    char *name;
    struct resource resource;
    unsigned int (*match)(struct base_network_interface *interface);
    unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer);

};

struct base_network_channel
{

    struct resource resource;
    unsigned int (*match)(struct base_network_interface *interface);
    void (*notify)(struct base_network_interface *interface);

};

void base_network_notify(struct base_network_interface *interface);
void base_network_registerinterface(struct base_network_interface *interface);
void base_network_registerinterfacenode(struct base_network_interfacenode *node);
void base_network_registerprotocol(struct base_network_protocol *protocol);
void base_network_registerchannel(struct base_network_channel *channel);
void base_network_unregisterinterface(struct base_network_interface *interface);
void base_network_unregisterinterfacenode(struct base_network_interfacenode *node);
void base_network_unregisterprotocol(struct base_network_protocol *protocol);
void base_network_unregisterchannel(struct base_network_channel *channel);
void base_network_initinterface(struct base_network_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*receive)(unsigned int count, void *buffer), unsigned int (*send)(unsigned int count, void *buffer), void *(*getpacket)(), void (*dumppacket)());
void base_network_initinterfacenode(struct base_network_interfacenode *node, struct base_network_interface *interface);
void base_network_initprotocol(struct base_network_protocol *protocol, char *name, unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*match)(struct base_network_interface *interface));
void base_network_initchannel(struct base_network_channel *channel, unsigned int (*match)(struct base_network_interface *interface), void (*notify)(struct base_network_interface *interface));
void base_network_setup();
