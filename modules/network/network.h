struct network_interface
{

    struct base_interface base;
    struct ctrl_networkctrl ctrl;
    unsigned int (*receive)(unsigned int count, void *buffer);
    unsigned int (*send)(unsigned int count, void *buffer);
    void *(*getpacket)();
    void (*dumppacket)();

};

struct network_interfacenode
{

    struct system_node base;
    struct system_node ctrl;
    struct system_node data;
    struct network_interface *interface;

};

struct network_protocol
{

    char *name;
    struct resource resource;
    unsigned int (*match)(struct network_interface *interface);
    unsigned int (*read)(struct network_interface *interface, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct network_interface *interface, unsigned int offset, unsigned int count, void *buffer);

};

struct network_channel
{

    struct resource resource;
    unsigned int (*match)(struct network_interface *interface);
    void (*notify)(struct network_interface *interface);
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct network_channelnode
{

    struct system_node base;
    struct system_node data;
    struct network_channel *channel;

};

void network_notify(struct network_interface *interface);
void network_registerinterface(struct network_interface *interface);
void network_registerinterfacenode(struct network_interfacenode *node);
void network_registerprotocol(struct network_protocol *protocol);
void network_registerchannel(struct network_channel *channel);
void network_registerchannelnode(struct network_channelnode *node);
void network_unregisterinterface(struct network_interface *interface);
void network_unregisterinterfacenode(struct network_interfacenode *node);
void network_unregisterprotocol(struct network_protocol *protocol);
void network_unregisterchannel(struct network_channel *channel);
void network_unregisterchannelnode(struct network_channelnode *node);
void network_initinterface(struct network_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*receive)(unsigned int count, void *buffer), unsigned int (*send)(unsigned int count, void *buffer), void *(*getpacket)(), void (*dumppacket)());
void network_initinterfacenode(struct network_interfacenode *node, struct network_interface *interface);
void network_initprotocol(struct network_protocol *protocol, char *name, unsigned int (*read)(struct network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*match)(struct network_interface *interface));
void network_initchannel(struct network_channel *channel, unsigned int (*match)(struct network_interface *interface), void (*notify)(struct network_interface *interface), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer));
void network_initchannelnode(struct network_channelnode *node, struct network_channel *channel);
