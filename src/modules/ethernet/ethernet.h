struct ethernet_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    struct system_node addr;
    unsigned int id;

};

void ethernet_notify(struct ethernet_interface *interface, void *buffer, unsigned int count);
void ethernet_registerinterface(struct ethernet_interface *interface);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id);
