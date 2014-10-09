struct base_mouse_interface
{

    struct base_interface base;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct base_mouse_node
{

    struct system_node base;
    struct system_node data;
    struct base_mouse_interface *interface;

};

void base_mouse_registerinterface(struct base_mouse_interface *interface);
void base_mouse_registernode(struct base_mouse_node *node);
void base_mouse_unregisterinterface(struct base_mouse_interface *interface);
void base_mouse_unregisternode(struct base_mouse_node *node);
void base_mouse_initinterface(struct base_mouse_interface *interface, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer));
void base_mouse_initnode(struct base_mouse_node *node, struct base_device *device, struct base_mouse_interface *interface);
void base_mouse_setup();
