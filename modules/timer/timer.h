struct timer_interface
{

    struct system_group base;
    unsigned int (*get_ticks)(struct base_device *device);
    void (*set_ticks)(struct base_device *device, unsigned int ticks);

};

void timer_register_device(struct timer_interface *interface, struct base_device *device);
void timer_init_interface(struct timer_interface *interface, unsigned int (*get_ticks)(struct base_device *device), void (*set_ticks)(struct base_device *device, unsigned int ticks));
