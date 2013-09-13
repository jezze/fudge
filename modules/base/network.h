struct base_network
{

    struct system_group base;
    unsigned int (*receive)(struct base_device *device, unsigned int count, void *buffer);
    unsigned int (*send)(struct base_device *device, unsigned int count, void *buffer);
    char mac[6];

};

void base_register_network(struct base_network *interface, struct base_device *device);
void base_init_network(struct base_network *interface, unsigned int (*receive)(struct base_device *device, unsigned int count, void *buffer), unsigned int (*send)(struct base_device *device, unsigned int count, void *buffer));
void base_setup_network();
