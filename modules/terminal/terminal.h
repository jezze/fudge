struct terminal_interface
{

    struct base_interface base;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct terminal_interfacenode
{

    struct system_node base;
    struct system_node data;
    struct terminal_interface *interface;

};

void terminal_registerinterface(struct terminal_interface *interface);
void terminal_registerinterfacenode(struct terminal_interfacenode *node);
void terminal_unregisterinterface(struct terminal_interface *interface);
void terminal_unregisterinterfacenode(struct terminal_interfacenode *node);
void terminal_initinterface(struct terminal_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer));
void terminal_initinterfacenode(struct terminal_interfacenode *node, struct terminal_interface *interface);
