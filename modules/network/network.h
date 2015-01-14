struct network_interfacenode
{

    struct system_node base;
    struct system_node ctrl;
    struct system_node data;
    struct network_interface *interface;

};

struct network_interface
{

    struct base_interface base;
    struct network_interfacenode node;
    struct ctrl_networksettings settings;
    unsigned int (*receive)(unsigned int count, void *buffer);
    unsigned int (*send)(unsigned int count, void *buffer);
    void *(*getpacket)();
    unsigned int (*copypacket)(unsigned int count, void *buffer);
    void (*dumppacket)();

};

struct network_protocol
{

    char *name;
    struct resource resource;

};

struct network_channelnode
{

    struct system_node base;
    struct system_node data;
    struct network_channel *channel;

};

struct network_channel
{

    struct resource resource;
    struct network_channelnode node;
    unsigned int (*match)(struct network_interface *interface, void *packet, unsigned int count);
    void (*notify)(struct network_interface *interface, void *packet, unsigned int count);
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);

};

void network_notify(struct network_interface *interface, void *packet, unsigned int count);
void network_registerinterface(struct network_interface *interface, struct base_bus *bus, unsigned int id);
void network_registerprotocol(struct network_protocol *protocol);
void network_registerchannel(struct network_channel *channel);
void network_unregisterinterface(struct network_interface *interface);
void network_unregisterprotocol(struct network_protocol *protocol);
void network_unregisterchannel(struct network_channel *channel);
void network_initinterface(struct network_interface *interface, struct base_driver *driver, unsigned int (*receive)(unsigned int count, void *buffer), unsigned int (*send)(unsigned int count, void *buffer), void *(*getpacket)(), unsigned int (*copypacket)(unsigned int count, void *buffer), void (*dumppacket)());
void network_initprotocol(struct network_protocol *protocol, char *name);
void network_initchannel(struct network_channel *channel, unsigned int (*match)(struct network_interface *interface, void *packet, unsigned int count), void (*notify)(struct network_interface *interface, void *packet, unsigned int count), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer));
