struct base_timer_interface
{

    unsigned int (*get_ticks)(struct base_device *device);
    void (*set_ticks)(struct base_device *device, unsigned int ticks);

};

void base_timer_register_interface(struct base_timer_interface *interface, struct base_device *device);
void base_timer_init_interface(struct base_timer_interface *interface, unsigned int (*get_ticks)(struct base_device *device), void (*set_ticks)(struct base_device *device, unsigned int ticks));
void base_timer_setup();
