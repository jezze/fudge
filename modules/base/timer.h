struct base_timer
{

    unsigned int (*get_ticks)(struct base_device *device);
    void (*set_ticks)(struct base_device *device, unsigned int ticks);

};

void base_init_timer(struct base_timer *interface, unsigned int (*get_ticks)(struct base_device *device), void (*set_ticks)(struct base_device *device, unsigned int ticks));
void base_setup_timer(struct system_group *root);
