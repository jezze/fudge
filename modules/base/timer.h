struct base_timer_interface
{

    struct base_interface base;
    void (*add_duration)(struct base_bus *bus, unsigned int id, unsigned int duration);

};

struct base_timer_node
{

    struct system_group base;
    struct base_device *device;
    struct base_timer_interface *interface;

};

void base_timer_register_interface(struct base_timer_interface *interface);
void base_timer_register_node(struct base_timer_node *node);
void base_timer_unregister_interface(struct base_timer_interface *interface);
void base_timer_unregister_node(struct base_timer_node *node);
void base_timer_init_interface(struct base_timer_interface *interface, void (*add_duration)(struct base_bus *bus, unsigned int id, unsigned int duration));
void base_timer_init_node(struct base_timer_node *node, struct base_device *device, struct base_timer_interface *interface);
void base_timer_setup();
