struct base_network_interface
{

    unsigned int (*receive)(struct base_device *device, unsigned int count, void *buffer);
    unsigned int (*send)(struct base_device *device, unsigned int count, void *buffer);
    unsigned char mac[6];

};

void base_network_register_interface(struct base_network_interface *interface, struct base_device *device);
void base_network_init_interface(struct base_network_interface *interface, unsigned int (*receive)(struct base_device *device, unsigned int count, void *buffer), unsigned int (*send)(struct base_device *device, unsigned int count, void *buffer));
void base_network_setup();
