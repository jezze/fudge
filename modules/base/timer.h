struct base_timer_interface
{

    struct base_interface base;
    void (*addduration)(struct base_bus *bus, unsigned int id, unsigned int duration);

};

struct base_timer_node
{

    struct system_node base;
    struct base_device *device;
    struct base_timer_interface *interface;

};

void base_timer_registerinterface(struct base_timer_interface *interface);
void base_timer_registernode(struct base_timer_node *node);
void base_timer_unregisterinterface(struct base_timer_interface *interface);
void base_timer_unregisternode(struct base_timer_node *node);
void base_timer_initinterface(struct base_timer_interface *interface, void (*addduration)(struct base_bus *bus, unsigned int id, unsigned int duration));
void base_timer_initnode(struct base_timer_node *node, struct base_device *device, struct base_timer_interface *interface);
void base_timer_setup();
