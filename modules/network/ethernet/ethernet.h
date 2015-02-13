struct ethernet_interfacenode
{

    struct system_node base;
    struct system_node ctrl;
    struct system_node data;
    struct ethernet_interface *interface;

};

struct ethernet_interface
{

    struct base_interface base;
    struct ctrl_networksettings settings;
    struct list mailboxes;
    unsigned int (*send)(unsigned int count, void *buffer);
    struct ethernet_interfacenode node;

};

struct ethernet_protocolnode
{

    struct system_node base;
    struct system_node data;
    struct ethernet_protocol *protocol;

};

struct ethernet_protocol
{

    struct resource resource;
    struct list mailboxes;
    struct list_item item;
    unsigned int type;
    void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer);
    struct ethernet_protocolnode node;

};

void ethernet_notify(struct ethernet_interface *interface, unsigned int count, void *buffer);
void ethernet_registerinterface(struct ethernet_interface *interface, struct base_bus *bus, unsigned int id);
void ethernet_registerprotocol(struct ethernet_protocol *protocol);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_unregisterprotocol(struct ethernet_protocol *protocol);
void ethernet_initinterface(struct ethernet_interface *interface, struct base_driver *driver, unsigned int (*send)(unsigned int count, void *buffer));
void ethernet_initprotocol(struct ethernet_protocol *protocol, char *name, unsigned short type, void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer));
