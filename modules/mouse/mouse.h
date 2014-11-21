struct mouse_interface
{

    struct base_interface base;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct mouse_interfacenode
{

    struct system_node base;
    struct system_node data;
    struct mouse_interface *interface;

};

void mouse_registerinterface(struct mouse_interface *interface);
void mouse_registerinterfacenode(struct mouse_interfacenode *node);
void mouse_unregisterinterface(struct mouse_interface *interface);
void mouse_unregisterinterfacenode(struct mouse_interfacenode *node);
void mouse_initinterface(struct mouse_interface *interface, struct base_driver *driver, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer));
void mouse_initinterfacenode(struct mouse_interfacenode *node, struct mouse_interface *interface);
