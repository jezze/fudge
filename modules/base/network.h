#define BASE_NETWORK_RESOURCE_PROTOCOL  0x8101

struct base_network_interface
{

    struct base_interface base;
    unsigned int (*receive)(struct base_bus *bus, unsigned int id, unsigned int count, void *buffer);
    unsigned int (*send)(struct base_bus *bus, unsigned int id, unsigned int count, void *buffer);
    void *(*getpacket)(struct base_bus *bus, unsigned int id);
    void (*dumppacket)(struct base_bus *bus, unsigned int id);
    unsigned char mac[6];

};

struct base_network_interfacenode
{

    struct system_node base;
    struct system_node data;
    struct system_node mac;
    struct base_device *device;
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

struct base_network_protocolnode
{

    struct system_node base;
    struct base_network_protocol *protocol;

};

void base_network_notify(struct base_network_interface *interface);
void base_network_registerinterface(struct base_network_interface *interface);
void base_network_registerinterfacenode(struct base_network_interfacenode *interfacenode);
void base_network_registerprotocol(struct base_network_protocol *protocol);
void base_network_registerprotocolnode(struct base_network_protocolnode *protocolnode);
void base_network_unregisterinterface(struct base_network_interface *interface);
void base_network_unregisterinterfacenode(struct base_network_interfacenode *interfacenode);
void base_network_unregisterprotocol(struct base_network_protocol *protocol);
void base_network_unregisterprotocolnode(struct base_network_protocolnode *protocolnode);
void base_network_initinterface(struct base_network_interface *interface, unsigned int (*receive)(struct base_bus *bus, unsigned int id, unsigned int count, void *buffer), unsigned int (*send)(struct base_bus *bus, unsigned int id, unsigned int count, void *buffer), void *(*getpacket)(struct base_bus *bus, unsigned int id), void (*dumppacket)(struct base_bus *bus, unsigned int id));
void base_network_initinterfacenode(struct base_network_interfacenode *interfacenode, struct base_device *device, struct base_network_interface *interface);
void base_network_initprotocol(struct base_network_protocol *protocol, char *name, unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*match)(struct base_network_interface *interface));
void base_network_initprotocolnode(struct base_network_protocolnode *protocolnode, struct base_network_protocol *protocol);
void base_network_setup();
