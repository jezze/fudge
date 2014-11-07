struct base_terminal_interface
{

    struct base_interface base;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct base_terminal_interfacenode
{

    struct system_node base;
    struct system_node data;
    struct base_terminal_interface *interface;

};

void base_terminal_registerinterface(struct base_terminal_interface *interface);
void base_terminal_registerinterfacenode(struct base_terminal_interfacenode *node);
void base_terminal_unregisterinterface(struct base_terminal_interface *interface);
void base_terminal_unregisterinterfacenode(struct base_terminal_interfacenode *node);
void base_terminal_initinterface(struct base_terminal_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer));
void base_terminal_initinterfacenode(struct base_terminal_interfacenode *node, struct base_terminal_interface *interface);
void base_terminal_setup();
