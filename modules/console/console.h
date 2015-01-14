struct console_interfacenode
{

    struct system_node base;
    struct system_node ctrl;
    struct system_node data;
    struct console_interface *interface;

};

struct console_interface
{

    struct base_interface base;
    struct console_interfacenode node;
    struct ctrl_consolesettings settings;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);

};

void console_registerinterface(struct console_interface *interface);
void console_unregisterinterface(struct console_interface *interface);
void console_initinterface(struct console_interface *interface, struct base_driver *driver, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer));
