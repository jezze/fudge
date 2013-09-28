struct base_clock_interface
{

    unsigned int timestamp;

};

void base_clock_register_interface(struct base_clock_interface *interface, struct base_device *device);
void base_clock_init_interface(struct base_clock_interface *interface);
void base_clock_setup();
