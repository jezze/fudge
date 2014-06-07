struct base_timer_interface
{

    struct base_interface base;
    unsigned int jiffies;
    void (*add_duration)(struct base_device *device, unsigned int duration);

};

void base_timer_register_interface(struct base_timer_interface *interface, struct base_device *device);
void base_timer_unregister_interface(struct base_timer_interface *interface);
void base_timer_init_interface(struct base_timer_interface *interface, void (*add_duration)(struct base_device *device, unsigned int duration));
void base_timer_setup();
