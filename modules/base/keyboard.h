struct base_keyboard_interface
{

    struct base_interface base;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct base_keyboard_node
{

    struct system_node base;
    struct system_node data;
    struct base_keyboard_interface *interface;

};

void base_keyboard_registerinterface(struct base_keyboard_interface *interface);
void base_keyboard_registernode(struct base_keyboard_node *node);
void base_keyboard_unregisterinterface(struct base_keyboard_interface *interface);
void base_keyboard_unregisternode(struct base_keyboard_node *node);
void base_keyboard_initinterface(struct base_keyboard_interface *interface, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer));
void base_keyboard_initnode(struct base_keyboard_node *node, struct base_device *device, struct base_keyboard_interface *interface);
void base_keyboard_setup();
