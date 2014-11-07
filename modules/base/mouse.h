struct base_mouse_interface
{

    struct base_interface base;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct base_mouse_interfacenode
{

    struct system_node base;
    struct system_node data;
    struct base_mouse_interface *interface;

};

void base_mouse_registerinterface(struct base_mouse_interface *interface);
void base_mouse_registerinterfacenode(struct base_mouse_interfacenode *node);
void base_mouse_unregisterinterface(struct base_mouse_interface *interface);
void base_mouse_unregisterinterfacenode(struct base_mouse_interfacenode *node);
void base_mouse_initinterface(struct base_mouse_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer));
void base_mouse_initinterfacenode(struct base_mouse_interfacenode *node, struct base_mouse_interface *interface);
void base_mouse_setup();
