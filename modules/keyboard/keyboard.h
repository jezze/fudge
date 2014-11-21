struct keyboard_interface
{

    struct base_interface base;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct keyboard_interfacenode
{

    struct system_node base;
    struct system_node data;
    struct keyboard_interface *interface;

};

void keyboard_registerinterface(struct keyboard_interface *interface);
void keyboard_registerinterfacenode(struct keyboard_interfacenode *node);
void keyboard_unregisterinterface(struct keyboard_interface *interface);
void keyboard_unregisterinterfacenode(struct keyboard_interfacenode *node);
void keyboard_initinterface(struct keyboard_interface *interface, struct base_driver *driver, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer));
void keyboard_initinterfacenode(struct keyboard_interfacenode *node, struct keyboard_interface *interface);
