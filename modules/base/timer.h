struct base_timer_interface
{

    struct base_interface base;
    void (*add_duration)(struct base_bus *bus, unsigned int id, unsigned int duration);

};

void base_timer_connect_interface(struct base_interface *interface, struct base_bus *bus, unsigned int id);
void base_timer_register_interface(struct base_timer_interface *interface);
void base_timer_unregister_interface(struct base_timer_interface *interface);
void base_timer_init_interface(struct base_timer_interface *interface, void (*add_duration)(struct base_bus *bus, unsigned int id, unsigned int duration));
void base_timer_setup();
