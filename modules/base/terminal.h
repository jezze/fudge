struct base_terminal_interface
{

    struct base_interface base;
    unsigned int (*rdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);

};

struct base_terminal_node
{

    struct system_node base;
    struct system_node data;
    struct base_device *device;
    struct base_terminal_interface *interface;

};

void base_terminal_registerinterface(struct base_terminal_interface *interface);
void base_terminal_registernode(struct base_terminal_node *node);
void base_terminal_unregisterinterface(struct base_terminal_interface *interface);
void base_terminal_unregisternode(struct base_terminal_node *node);
void base_terminal_initinterface(struct base_terminal_interface *interface, unsigned int (*rdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer));
void base_terminal_initnode(struct base_terminal_node *node, struct base_device *device, struct base_terminal_interface *interface);
void base_terminal_setup();
