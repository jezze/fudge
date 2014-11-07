struct base_keyboard_interface
{

    struct base_interface base;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct base_keyboard_interfacenode
{

    struct system_node base;
    struct system_node data;
    struct base_keyboard_interface *interface;

};

void base_keyboard_registerinterface(struct base_keyboard_interface *interface);
void base_keyboard_registerinterfacenode(struct base_keyboard_interfacenode *node);
void base_keyboard_unregisterinterface(struct base_keyboard_interface *interface);
void base_keyboard_unregisterinterfacenode(struct base_keyboard_interfacenode *node);
void base_keyboard_initinterface(struct base_keyboard_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer));
void base_keyboard_initinterfacenode(struct base_keyboard_interfacenode *node, struct base_keyboard_interface *interface);
void base_keyboard_setup();
