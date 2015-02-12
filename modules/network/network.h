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
    struct list mailboxes;
    unsigned int (*rdata)(unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int count, void *buffer);

};

struct network_protocolnode
{

    struct system_node base;
    struct system_node data;
    struct network_protocol *protocol;

};

struct network_protocol
{

    struct resource resource;
    struct network_protocolnode node;
    struct list_item item;
    struct list mailboxes;
    unsigned int type;
    void (*notify)(unsigned int count, void *buffer);

};

void network_notify(struct network_interface *interface, unsigned int count, void *buffer);
void network_registerinterface(struct network_interface *interface, struct base_bus *bus, unsigned int id);
void network_registerprotocol(struct network_protocol *protocol);
void network_unregisterinterface(struct network_interface *interface);
void network_unregisterprotocol(struct network_protocol *protocol);
void network_initinterface(struct network_interface *interface, struct base_driver *driver, unsigned int (*rdata)(unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int count, void *buffer));
void network_initprotocol(struct network_protocol *protocol, char *name, unsigned short type, void (*notify)(unsigned int count, void *buffer));
