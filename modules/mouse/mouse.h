struct mouse_interfacenode
{

    struct system_node base;
    struct system_node data;
    struct mouse_interface *interface;

};

struct mouse_interface
{

    struct base_interface base;
    struct mouse_interfacenode node;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);

};

void mouse_registerinterface(struct mouse_interface *interface, struct base_bus *bus, unsigned int id);
void mouse_unregisterinterface(struct mouse_interface *interface);
void mouse_initinterface(struct mouse_interface *interface, struct base_driver *driver, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer));
