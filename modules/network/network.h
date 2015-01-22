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
    unsigned int (*rdata)(unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int count, void *buffer);

};

void network_notify(unsigned int count, void *buffer);
void network_registerinterface(struct network_interface *interface, struct base_bus *bus, unsigned int id);
void network_unregisterinterface(struct network_interface *interface);
void network_initinterface(struct network_interface *interface, struct base_driver *driver, unsigned int (*rdata)(unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int count, void *buffer));
