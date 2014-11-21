struct console_interface
{

    struct base_interface base;
    struct ctrl_consolectrl ctrl;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct console_interfacenode
{

    struct system_node base;
    struct system_node ctrl;
    struct system_node data;
    struct console_interface *interface;

};

void console_registerinterface(struct console_interface *interface);
void console_registerinterfacenode(struct console_interfacenode *node);
void console_unregisterinterface(struct console_interface *interface);
void console_unregisterinterfacenode(struct console_interfacenode *node);
void console_initinterface(struct console_interface *interface, struct base_driver *driver, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer));
void console_initinterfacenode(struct console_interfacenode *node, struct console_interface *interface);
